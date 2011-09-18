using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics.Contracts;
using System.Diagnostics;

namespace Restify
{
    [DebuggerDisplay("{ToString(), nq}")]
    public struct Base16
    {
        static char ToNibble(int value)
        {
            return (char)(value < 10 ? '0' + value : 'A' + value - 10);
        }

        public static string Encode(byte[] bytes)
        {
            Contract.Requires(bytes != null);

            var sb = new StringBuilder();
            for (int i = 0; i < bytes.Length; i++)
            {
                sb.Append(ToNibble(bytes[i] >> 4));
                sb.Append(ToNibble(bytes[i] & 15));
            }
            return sb.ToString();
        }

        static int FromNibble(char value)
        {
            return value <= '9' ? value - '0' : 10 + value - 'A';
        }

        public static byte[] Decode(string s)
        {
            Contract.Requires(s != null);
            Contract.Requires(s.Length > 0);
            Contract.Requires((s.Length % 2) == 0);

            var bytes = new byte[s.Length / 2];

            for (int i = 0; i < bytes.Length; i++)
                bytes[i] = (byte)((FromNibble(s[i << 1]) << 4) | FromNibble(s[(i << 1) + 1]));

            return bytes;
        }

        byte[] bytes;

        public Base16(string s)
        {
            this.bytes = Decode(s);
        }

        public Base16(byte[] bytes)
        {
            Contract.Requires(bytes != null);

            var copy = new byte[bytes.Length];
            Buffer.BlockCopy(bytes, 0, copy, 0, copy.Length);
            this.bytes = copy;
        }

        public byte[] ToByteArray()
        {
            var copy = new byte[bytes.Length];
            Buffer.BlockCopy(bytes, 0, copy, 0, copy.Length);
            return copy;
        }

        public override string ToString()
        {
            if (bytes != null)
                return Encode(bytes);
            
            return null;
        }
    }
}
