using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel.Composition;
using Restify.ServiceModel.Composition;
using System.Diagnostics;
using Restify.Server;

namespace Restify.ServiceModel
{
    [ComposableServiceExport("/restify/master/queue", typeof(QueueService))]
    class QueueService : IQueueService
    {
        static Queue<string> pq = new Queue<string>();
        static HashSet<string> pqset = new HashSet<string>(StringComparer.Ordinal);

        private IClientContainer container;

        [ImportingConstructor]
        public QueueService(IClientContainer container)
        {
            this.container = container;
        }

        public void Enqueue(string trackId)
        {
            lock (pq)
            {
                // only accept currently not queued tracks
                if (pqset.Add(trackId))
                {
                    pq.Enqueue(trackId);
                    Trace.WriteLine(string.Format("Enqueued '{0}'", trackId));
                }
                else
                    return;
            }
            container.Play(); // TODO: this is a wrong, as we only want to play if we queue up a song for the first time
        }

        public string Dequeue()
        {
            lock (pq)
            {
                if (pq.Count > 0)
                {
                    var trackId = pq.Dequeue();
                    pqset.Remove(trackId);
                    Trace.WriteLine(string.Format("Dequeued '{0}'", trackId));
                    return trackId;
                }
            }
            return null;
        }

        void IDisposable.Dispose()
        {
        }
    }
}
