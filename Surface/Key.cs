using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace watchasayin
{
    /// <summary>
    /// The different types of cryptographic algorithms.
    /// </summary>
    public enum TCryptoAlgo
    {
        /// <summary>
        /// The AES algorithm.
        /// </summary>
        AES = 0,
        /// <summary>
        /// An unknown algorithm.
        /// </summary>
        unknown
    }

    /// <summary>
    /// Class representing a cryptographic key.
    /// </summary>
    public class CryptoKey
    {
        private TCryptoAlgo algo;
        private byte[] rawKey;

        /// <summary>
        /// The public constructor.
        /// </summary>
        /// <param name="rawKey">The raw key material.</param>
        /// <param name="algo">The cryptoggraphic algorithm the key is intended to be used with.</param>
        public CryptoKey(byte[] rawKey, TCryptoAlgo algo)
        {
            this.algo = algo;
            this.rawKey = rawKey;
        }

        /// <summary>
        /// Gets the raw key material.
        /// </summary>
        /// <returns>The raw key.</returns>
        public byte[] getRawKey()
        {
            return rawKey;
        }

        /// <summary>
        /// The bit-strength of the key.
        /// </summary>
        public int bitStrength { get { return rawKey.Length * 8; } }

        /// <summary>
        /// The cryptoggraphic algorithm the key is intended to be used with.
        /// </summary>
        public TCryptoAlgo algorithm { get { return algo; } }
    }
}
