using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Numerics;

namespace Restify
{
    public struct Ascii85 : IEquatable<Ascii85>
    {
        public static readonly EncodingScheme EncodingScheme = new EncodingScheme(
                (from c in Enumerable.Range('0', 10) select (char)c)
                .Concat((from c in Enumerable.Range('A', 26) select (char)c))
                .Concat((from c in Enumerable.Range('a', 26) select (char)c))
                .Concat(from c in "!#$%&()*+-;<=>?@^_`{|}~" select c)
                .ToArray()
            );

        private byte[] buffer;

        public Ascii85(string s)
        {
            this.buffer = EncodingScheme.Decode(s);
        }

        public Ascii85(byte[] buffer)
        {
            this.buffer = buffer;
        }

        public override bool Equals(object obj)
        {
            var value = obj as Ascii85?;
            if (value.HasValue)
            {
                return Equals(value.Value);
            }
            return false;
        }

        public bool Equals(Ascii85 other)
        {
            return this.buffer.SequenceEqual(other.buffer);
        }

        public override int GetHashCode()
        {
            // 32-bit FNV-1 hash
            int hashCode = -2128831035;
            for (int i = 0; i < buffer.Length; i++)
            {
                hashCode *= 16777619;
                hashCode ^= buffer[i];
            }
            return hashCode;
        }

        public override string ToString()
        {
            return EncodingScheme.Encode(buffer);
        }
    }
}
