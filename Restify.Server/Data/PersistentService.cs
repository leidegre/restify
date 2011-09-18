using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel.Composition;
using System.Security.Cryptography;
using System.Diagnostics.Contracts;
using System.Data.SqlServerCe;

namespace Restify.Data
{
    [Export(typeof(IPersistentService))]
    [PartCreationPolicy(CreationPolicy.Shared)]
    class PersistentService : IPersistentService
    {
        static byte[] CreateUserId(string userName)
        {
            Contract.Requires(userName != null);
            var s = userName.Trim().ToUpperInvariant();
            using (var hashServiceProvider = new SHA256CryptoServiceProvider())
            {
                var bytes = Encoding.UTF8.GetBytes(s);
                return hashServiceProvider.ComputeHash(bytes);
            }
        }

        public IEnvironment Host { get; private set; }

        [ImportingConstructor]
        public PersistentService(IEnvironment host)
        {
            this.Host = host;
        }

        SqlCeConnectionStringBuilder CreateConnectionString()
        {
            var connBuilder = new SqlCeConnectionStringBuilder();
            connBuilder.DataSource = Host.GetVirtualPath("restify.sdf").Path;
            return connBuilder;
        }

        SqlCeConnection CreateConnection()
        {
            var connBuilder = CreateConnectionString();
            return new SqlCeConnection(connBuilder.ConnectionString);
        }

        public void Initialize()
        {
            var fullDbPath = Host.GetVirtualPath("restify.sdf");
            if (Host.GetFile(fullDbPath).IsValid)
            {
                var connBuilder = CreateConnectionString();

                using (var engine = new SqlCeEngine(connBuilder.ConnectionString))
                {
                    engine.CreateDatabase();
                }

                using (var conn = new SqlCeConnection(connBuilder.ConnectionString))
                {
                    conn.Open();
                    
                    foreach (var schemaFile in Host.GetFiles(Host.GetVirtualPath(@"Data\Schema")).Where(x => x.Path.EndsWith(".sql", StringComparison.OrdinalIgnoreCase)).OrderBy(x => x))
                    {
                        var cmd = conn.CreateCommand();
                        cmd.CommandText = Host.GetText(schemaFile);
                        cmd.ExecuteNonQuery();
                    }
                }
            }
        }

        public void Enqueue(string userName, string trackId)
        {
            var userId = CreateUserId(userName);

            using (var conn = CreateConnection())
            {
                var cmd = conn.CreateCommand();

                cmd.CommandText = @"insert into [Queue] ([Timestamp], [TrackId], [UserId]) values (@Timestamp, @TrackId, @UserId);";

                cmd.Parameters.AddWithValue("@Timestamp", DateTime.UtcNow.Ticks);
                cmd.Parameters.AddWithValue("@TrackId", trackId);
                cmd.Parameters.AddWithValue("@UserId", userId);

                cmd.ExecuteNonQuery();
            }
        }

        public string Dequeue()
        {
            using (var conn = CreateConnection())
            {
                string trackId = null;
                long timestamp = 0;

                using (var cmd = conn.CreateCommand())
                {
                    cmd.CommandText = @"select top (1) [Timestamp], [TrackId] from [Queue] order by [Timestamp], [TrackId];";

                    using (var reader = cmd.ExecuteReader())
                    {
                        if (reader.Read())
                        {
                            timestamp = reader.GetInt64(0);
                            trackId = reader.GetString(1);
                        }
                    }
                }

                if (trackId != null)
                {
                    using (var cmd = conn.CreateCommand())
                    {
                        cmd.CommandText = @"delete from [Queue] where [Timestamp] = @Timestamp and [TrackId] = @TrackId";
                        
                        cmd.Parameters.AddWithValue("@Timestamp", timestamp);
                        cmd.Parameters.AddWithValue("@TrackId", trackId);

                        cmd.ExecuteNonQuery();
                    }
                }

                return trackId;
            }
        }
    }
}
