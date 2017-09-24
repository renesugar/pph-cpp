// /*
//  * fnv - Fowler/Noll/Vo- hash code
//  *
//  * @(#) $Revision: 5.4 $
//  * @(#) $Id: fnv.h,v 5.4 2009/07/30 22:49:13 chongo Exp $
//  * @(#) $Source: /usr/local/src/cmd/fnv/RCS/fnv.h,v $
//  *
//  ***
//  *
//  * Fowler/Noll/Vo- hash
//  *
//  * The basis of this hash algorithm was taken from an idea sent
//  * as reviewer comments to the IEEE POSIX P1003.2 committee by:
//  *
//  *      Phong Vo (http://www.research.att.com/info/kpv/)
//  *      Glenn Fowler (http://www.research.att.com/~gsf/)
//  *
//  * In a subsequent ballot round:
//  *
//  *      Landon Curt Noll (http://www.isthe.com/chongo/)
//  *
//  * improved on their algorithm.  Some people tried this hash
//  * and found that it worked rather well.  In an EMail message
//  * to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
//  *
//  * FNV hashes are designed to be fast while maintaining a low
//  * collision rate. The FNV speed allows one to quickly hash lots
//  * of data while maintaining a reasonable collision rate.  See:
//  *
//  *      http://www.isthe.com/chongo/tech/comp/fnv/index.html
//  *
//  * for more details as well as other forms of the FNV hash.
//  *
//  ***
//  *
//  * NOTE: The FNV-0 historic hash is not recommended.  One should use
//  *	 the FNV-1 hash instead.
//  *
//  * To use the 32 bit FNV-0 historic hash, pass FNV0_32_INIT as the
//  * Fnv32_t hashval argument to fnv_32_buf() or fnv_32_str().
//  *
//  * To use the 64 bit FNV-0 historic hash, pass FNV0_64_INIT as the
//  * Fnv64_t hashval argument to fnv_64_buf() or fnv_64_str().
//  *
//  * To use the recommended 32 bit FNV-1 hash, pass FNV1_32_INIT as the
//  * Fnv32_t hashval argument to fnv_32_buf() or fnv_32_str().
//  *
//  * To use the recommended 64 bit FNV-1 hash, pass FNV1_64_INIT as the
//  * Fnv64_t hashval argument to fnv_64_buf() or fnv_64_str().
//  *
//  * To use the recommended 32 bit FNV-1a hash, pass FNV1_32A_INIT as the
//  * Fnv32_t hashval argument to fnv_32a_buf() or fnv_32a_str().
//  *
//  * To use the recommended 64 bit FNV-1a hash, pass FNV1A_64_INIT as the
//  * Fnv64_t hashval argument to fnv_64a_buf() or fnv_64a_str().
//  *
//  ***
//  *
//  * Please do not copyright this code.  This code is in the public domain.
//  *
//  * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
//  * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
//  * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
//  * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
//  * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
//  * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
//  * PERFORMANCE OF THIS SOFTWARE.
//  *
//  * By:
//  *	chongo <Landon Curt Noll> /\oo/\
//  *      http://www.isthe.com/chongo/
//  *
//  * Share and Enjoy!	:-)
//  */

#ifndef _FNV64A_HASH_H
#define _FNV64A_HASH_H

static constexpr uint64_t FNV1A_64_INIT = UINT64_C(0xcbf29ce484222325);
static constexpr uint64_t FNV_64_PRIME  = UINT64_C(0x100000001b3);

uint64_t fnv64a_hash(const std::string& str, uint64_t multiplier, uint64_t adjustment) {
  uint64_t hval = FNV1A_64_INIT;

  //
  // FNV-1a hash each octet of the buffer
  //
  for (int i = 0; i < str.size(); i++) {

    // xor the bottom with the current octet
    hval ^= static_cast<uint64_t>(str[i]);

    // multiply by the 64 bit FNV magic prime mod 2^64
    hval *= FNV_64_PRIME;
  }

  // return our new hash value
  return hval + adjustment;
}

#endif  // _FNV64A_HASH_H
