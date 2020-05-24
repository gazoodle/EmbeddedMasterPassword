///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  test.cpp - Host application for running unit test suite
//
//      I *LOVE* unit tests. I *LOVE* test driven development. This whole suite has been built
//      using these techniques. I especially love that they give you complete confidence to refactor
//      code to your hearts desire in the secure knowledge that the code is still performing as you
//      expect. This was actually intrumental in getting a working sparse ROMix function to work
//      for embedded systems, without which this project wouldn't really be feasable.
//
//  Copyright (C) 2020, Gazoodle (https://github.com/gazoodle)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#define TEST_SUITE

#include <stdio.h>
#include <stdint.h>
#include <sha256.h>
#include <hmac.h>
#include <pbkdf2.h>
#include <scrypt.h>
#include <mpw.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Foward declarations of test functions
void test_sha256(void);
void test_hmac_sha256(void);
void test_pbkdf2_hmac_sha256(void);
void test_scrypt(void);
void test_MPW(void);
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    printf("### Embedded Master Password Unit Test Suite ###\n\n");

    printf("SHA256 tests **********************************************\n");
    test_sha256();
    printf("HMAC-SHA256 tests *****************************************\n");
    test_hmac_sha256();
    printf("PBKDF2-HMAC-SHA256 tests **********************************\n");
    test_pbkdf2_hmac_sha256();    
    printf("scrypt tests **********************************************\n");
    test_scrypt();
	printf("MasterPassword tests **************************************\n");
    test_MPW();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//      General support for test suite
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
void assert(const uint8_t val, const uint8_t expected, const char * ctx)
{
	if ( val != expected ) 
	{
        printf("Assertion failed. %s. Found 0x%.02x expected 0x%.02x\n", ctx, val, expected );
		exit(1);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void assert(const size_t val, const size_t expected, const char * ctx)
{
	if ( val != expected ) 
	{
        printf("Assertion failed. %s. Found 0x%.02lx expected 0x%.02lx\n", ctx, val, expected );
		exit(1);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t H2I(const char c)
{
	if ((c >= '0') && ( c <= '9' ))
		return c - '0';
	return 10 + c - 'a';
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void assert_hash(const uint8_t *hash, const char *expected, const char *test, uint8_t hash_len)
{
	assert( strlen(expected), (size_t)(hash_len*2), "Length of `expected` string is incorrect");
	for(int i=0;i<hash_len;i++)
	{
		// Allow test cases to specified indeterminate values
		if(expected[i<<1] == '?')
			continue;
		uint8_t expected_byte = (H2I(expected[i<<1])<<4) | (H2I(expected[(i<<1)+1]));
		char ctx_buf[80];
		sprintf(ctx_buf, "%s hash @ %d", test, i);
		assert(hash[i], expected_byte, ctx_buf);
	}

	printf("Test [%s] passed\n", test);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//      SHA256 suite
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
void assert_sha256(const char* message, const char* expected, const char * test_name)
{
    SHA256 sha256(message);
    const uint8_t * hash = sha256.digest();
    assert_hash( hash, expected, test_name, SHA256::HASH_SIZE_BYTES );
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void test_sha256(void)
{
    // Empty string test vector from https://en.wikipedia.org/wiki/SHA-2#Test_vectors
    assert_sha256("", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855", "Empty string");
    // https://csrc.nist.gov/csrc/media/publications/fips/180/2/archive/2002-08-01/documents/fips180-2.pdf B.1
    assert_sha256("abc", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "FIPS 180-2 B.1");
    // https://csrc.nist.gov/csrc/media/publications/fips/180/2/archive/2002-08-01/documents/fips180-2.pdf B.2
    assert_sha256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1", "FIPS 180-2 B.2");
    // https://csrc.nist.gov/csrc/media/publications/fips/180/2/archive/2002-08-01/documents/fips180-2.pdf B.3
    SHA256 sha256;
    for(int i=0;i<1000000;i++)
        sha256.enqueue('a');
    assert_hash( sha256.digest(), "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0", "FIPS 180-2 B.3", SHA256::HASH_SIZE_BYTES);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//      HMAX_SHA256 suite
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
void assert_hmac_sha256(const char* key, const char *message, const char *expected, const char *test_name)
{
	HMAC<SHA256> hmac_sha256( key, message );
	const uint8_t * hash = hmac_sha256.digest();
	assert_hash( hash, expected, test_name, SHA256::HASH_SIZE_BYTES );
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void test_hmac_sha256(void)
{
    // Empty key & message (empirical test)
    assert_hmac_sha256("", "", "b613679a0814d9ec772f95d778c35fc5ff1697c493715653c6c712144292c5ad", "Empty key & message");
    // From https://en.wikipedia.org/wiki/HMAC#Examples
    assert_hmac_sha256("key", "The quick brown fox jumps over the lazy dog", "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8", "Wikipedia example");
    // From https://tools.ietf.org/html/rfc4231#section-4.2
    assert_hmac_sha256("\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b", "Hi There", "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7", "RFC 4231 - Test Case #1");
    // From https://tools.ietf.org/html/rfc4231#section-4.3
    assert_hmac_sha256("Jefe", "what do ya want for nothing?", "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843", "RFC 4231 - Test Case #2");
    // From https://tools.ietf.org/html/rfc4231#section-4.4
    assert_hmac_sha256("\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa", "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd", "773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe", "RFC 4231 - Test Case #3");
    // From https://tools.ietf.org/html/rfc4231#section-4.5
    assert_hmac_sha256("\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19", "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd", "82558a389a443c0ea4cc819899f2083a85f0faa3e578f8077a2e3ff46729665b", "RFC 4231 - Test Case #4");
    // From https://tools.ietf.org/html/rfc4231#section-4.6
    assert_hmac_sha256("\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c", "Test With Truncation", "a3b6167473100ee06e0c796c2955552b????????????????????????????????", "RFC 4231 - Test Case #5");
    // From https://tools.ietf.org/html/rfc4231#section-4.7
    assert_hmac_sha256("\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa", "Test Using Larger Than Block-Size Key - Hash Key First", "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54", "RFC 4231 - Test Case #6");
    // From https://tools.ietf.org/html/rfc4231#section-4.8
    assert_hmac_sha256("\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa\x0aa", "This is a test using a larger than block-size key and a larger than block-size data. The key needs to be hashed before being used by the HMAC algorithm.", "9b09ffa71b942fcb27635fbcd5b0e944bfdc63644f0713938a7f51535c3a35e2", "RFC 4231 - Test Case #7");    
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//      PBKDF2_HMAX_SHA256 suite
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
template<uint32_t dkLen>
void assert_pkbdf2_hmac_sha256(const char *password, const char *salt, uint32_t c, const char *expected, const char *test_name )
{
	PBKDF2<HMAC<SHA256>, dkLen> pbkdf2_hmac_sha256(password, salt, c );
	assert_hash( pbkdf2_hmac_sha256.result(), expected, test_name, dkLen );
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void test_pbkdf2_hmac_sha256(void)
{
    // Various unvalidated tests from https://stackoverflow.com/questions/5130513/pbkdf2-hmac-sha2-test-vectors
    assert_pkbdf2_hmac_sha256<32>("password", "salt", 1, "120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b", "Unknown #1");
    assert_pkbdf2_hmac_sha256<32>("password", "salt", 2, "ae4d0c95af6b46d32d0adff928f06dd02a303f8ef3c251dfd6e2d85a95474c43", "Unknown #2");
    assert_pkbdf2_hmac_sha256<32>("password", "salt", 4096, "c5e478d59288c841aa530db6845c4c8d962893a001ce4e11a4963873aa98134a", "Unknown #3");
    // Unknown#4 ignored as it has 2^32 iterations and I'm not about to wait for that ;-)
    assert_pkbdf2_hmac_sha256<40>("passwordPASSWORDpassword", "saltSALTsaltSALTsaltSALTsaltSALTsalt", 4096, "348c89dbcbd32b2f32d814b8116e84cf2b17347ebc1800181c4e2a1fb8dd53e1c635518c7dac47e9", "Unknown #5");
    // https://tools.ietf.org/html/rfc7914#page-12 #1
    assert_pkbdf2_hmac_sha256<64>("passwd", "salt", 1, "55ac046e56e3089fec1691c22544b605f94185216dde0465e68b9d57c20dacbc49ca9cccf179b645991664b39d77ef317c71b845b1e30bd509112041d3a19783", "RFC7914 Test #1" );
    // https://tools.ietf.org/html/rfc7914#page-12 #2
    assert_pkbdf2_hmac_sha256<64>("Password", "NaCl", 80000, "4ddcd8f60b98be21830cee5ef22701f9641a4418d04c0414aeff08876b34ab56a1d425a1225833549adb841b51c9b3176a272bdebba1d078478f62b397f33c8d", "RFC7914 Test #2" );
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//      scrypt suite
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
void test_scrypt(void)
{
    scrypt_mixer<16,1,1,64,0,0> mixer;

	// From http://cr.yp.to/snuffle/spec.pdf
    Salsa20Block in = {0};
	mixer.Salsa20( in, 20 );
	assert_hash( reinterpret_cast<uint8_t*>(&in), "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", "Salsa20 Test vector #1", sizeof(in));

	uint8_t in1[] = {211,159,13,115,76,55,82,183,3,117,222,37,191,187,234,136,49,237,179,48,1,106,178,219,175,199,166,48,86,16,179,207,31,240,32,63,15,83,93,161,116,147,48,113,238,55,204,36,79,201,235,79,3,81,156,47,203,26,244,243,88,118,104,54};
    char* p = reinterpret_cast<char *>(in1);
	mixer.Salsa20( reinterpret_cast<Salsa20Block &>(*p), 20 );
	uint8_t exp1[] = {109,42,178,168,156,240,248,238,168,196,190,203,26,110,170,154,29,29,150,26,150,30,235,249,190,163,251,48,69,144,51,57,118,40,152,157,180,57,27,94,107,42,236,35,27,111,114,114,219,236,232,135,111,155,110,18,24,232,95,158,179,19,48,202};
	char exp1_hex[128+1];
	for(int i=0;i<64;i++)
		sprintf(&exp1_hex[i<<1],"%.02x",exp1[i]);
	assert_hash(in1,exp1_hex, "Salsa20 Test vector #2", sizeof(in1));

	scrypt<16,1,1,64> scrypt1;
	const uint8_t * r = scrypt1.hash( "", "", 0 );
    assert_hash(r, "77d6576238657b203b19ca42c18a0497f16b4844e3074ae8dfdffa3fede21442fcd0069ded0948f8326a753a0fc81f17e8d3e0fb2e0d3628cf35e20c38d18906", "scrypt #1", 64 );

	scrypt<16,8,2,64> scrypt2;
    r = scrypt2.hash( "", "", 0 );
    assert_hash(r, "8d12c62f0dab079dcb95b698a5012d79cf25ae9f6a2e2990f797ea92bcb907a656f1d3c886b0f1c725e42adcc54713fb514d2e070ea3070a4cfcd6c877a364b8", "scrypt #2", 64 );

	scrypt<32768,8,2,64> scrypt3;
    r = scrypt3.hash( "", "", 0 );
    assert_hash(r, "dbf4a1bef9c302095a55b12c6901c42187774dd8d51f1444a43244710cd127905db9afdded6e233b2afbddd5003d383538d23cbf997325e21068977fc6d740f5", "scrypt #3", 64 );
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//      MPW suite
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    const char *            user;
    const char *            password;
    const char *            site;
    uint32_t                counter;
    MPM_Password_Type       type;
    const char *            expected;
} MPW_Test_Data;
///////////////////////////////////////////////////////////////////////////////////////////////////
MPW_Test_Data test_data[] = {
    //  user                    password                    site                        count       type            expected
    {   "user",                 "password",                 "example.com",              1,          Long,           "ZedaFaxcZaso9*" },
    {   0,                      0,                          0,                          0,          Phrase,         "ze juzxo sax taxocre" },
    {   0,                      0,                          0,                          0,          Name,           "zedjuzoco" },
    {   0,                      0,                          0,                          0,          Maximum,        "pf4zS1LjCg&LjhsZ7T2~" },
    {   0,                      0,                          0,                          0,          Medium,         "ZedJuz8$" },
    {   0,                      0,                          0,                          0,          Basic,          "pIS54PLs" },
    {   0,                      0,                          0,                          0,          Short,          "Zed5" },
    {   0,                      0,                          0,                          0,          PIN,            "6685" },
    {   0,                      0,                          0,                          2,          Long,           "Fovi2@JifpTupx" },
    {   0,                      0,                          0,                          3,          Long,           "KizcQuho9[Xicu" },
    {   0,                      0,                          0,                          4,          Long,           "DoztXidwBogi1]" },
    {   0,                      0,                          0,                          40,         Long,           "Kozt3;DiduKagq" },

    {   "once",                 "twice",                    "three",                    1,          Long,           "Bopt6[PakaQile" },

    {   "Robert Lee Mitchell",  "banana colored duckling",  "masterpasswordapp.com",    1,          Long,           "Jejr5[RepuSosp" },
    {   0,                      0,                          0,                          0,          Maximum,        "W6@692^B1#&@gVdSdLZ@" },
    {   0,                      0,                          0,                          0,          Medium,         "Jej2$Quv" },
    {   0,                      0,                          0,                          0,          Basic,          "WAo2xIg6" },
    {   0,                      0,                          0,                          0,          Short,          "Jej2" },
    {   0,                      0,                          0,                          0,          PIN,            "7662" },
    {   0,                      0,                          0,                          0,          Name,           "jejraquvo" },
    {   0,                      0,                          0,                          0,          Phrase,         "jejr quv cabsibu tam" },
    {   0,                      0,                          "twitter.com",              1,          Long,           "PozoLalv0_Yelo" },

#ifdef ENABLE_MPW_EXTENSIONS
    // Non-standard tests and extensions
    {   "user",                 "password",                 "example.com",              1,          PIN_Six,        "668545" },
    {   0,                      0,                          0,                          1,          Vast,           "pf4zS1LjCg&LjhsZ7T6p(nC&cwLM7#" },
    {   0,                      0,                          0,                          1,          BigPhrase,      "ze juzxo sax taxocre zeswojojo jiv sec" },
#endif
};
///////////////////////////////////////////////////////////////////////////////////////////////////
void test_MPW(void)
{
    MPW_Test_Data   td = {0};
    MPW             mpw;

    // Run over the test matrix, gather the test data into a local copy...
    for(unsigned int i=0; i<countof(test_data); i++)
    {
        // Should the Master Password generator be logged in ?
        bool login = ( test_data[i].user != 0) | ( test_data[i].password != 0 );
        // Should the Master Password generator be initialized ?
        bool init = ( test_data[i].site != 0 ) | ( test_data[i].counter != 0 );

        td.user     = test_data[i].user ? test_data[i].user : td.user;
        td.password = test_data[i].password ? test_data[i].password : td.password;
        td.site     = test_data[i].site ? test_data[i].site : td.site;
        td.counter  = test_data[i].counter ? test_data[i].counter : td.counter;
        td.type     = test_data[i].type;
        td.expected = test_data[i].expected;

        if ( login )
            mpw.login(td.user, td.password, 0);
        if ( login || init )
            mpw.site(td.site, td.counter);
        
        const char* password = mpw.generate(td.type);
        bool matched = strcmp( password, td.expected ) == 0;

        printf("Test %.2d: User(%s,%s) -> Site(%s,%d) -> Password(%d) == `%s` [Expected `%s`] -> %s\n",
            i+1,
            td.user, td.password,
            td.site, td.counter,
            td.type,
            password,
            td.expected,
            matched ? "✓" : "❎"
            );

        if ( !matched )
        {
            printf("^^^^^^^^ DID NOT MATCH ^^^^^^^^^^^\n");
            exit(1);
        }
    }

    printf("+=================================================+\n");
    printf("|                                                 |\n");
    printf("|         MasterPassword Tests Complete           |\n");
    printf("|                                                 |\n");
    printf("+=================================================+\n");
}
///////////////////////////////////////////////////////////////////////////////////////////////////
