using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Numerics;

namespace Restify
{
    // This is just a general purpose encoder/decoder for arbitrary alphabets
    // it's slow (based on BigInteger and Dictionary) but convenient and simple

    public class EncodingScheme
    {
        private char[] encodingArray;
        private Dictionary<char, int> decodingArray;
        private BigInteger numBase;

        public EncodingScheme(char[] alphabet)
        {
            encodingArray = alphabet;
            
            decodingArray = new Dictionary<char, int>();
            for (int i = 0; i < alphabet.Length; i++)
                decodingArray[alphabet[i]] = i;

            numBase = alphabet.Length;
        }

        public string Encode(byte[] buffer)
        {
            var num = new BigInteger(buffer);
            var sb = new StringBuilder();
            while (!num.IsZero)
            {
                BigInteger rem;
                num = BigInteger.DivRem(num, numBase, out rem);
                sb.Append(encodingArray[(int)rem]);
            }
            return sb.ToString();
        }

        public byte[] Decode(string s)
        {
            var num = new BigInteger();
            for (int i = 0; i < s.Length; i++)
            {
                num += BigInteger.Pow(numBase, i) * decodingArray[s[i]];
            }
            return num.ToByteArray();
        }
    }
}
