#include "url.h"
#include "gtest/gtest.h"

#define NMEM(array) (sizeof(array)/sizeof(array[0]))

struct UrlCase {
    const char *input;
    bool is_valid;
    const char *scheme;
    const char *username;
    const char *password;
    const char *host;
    int port;
    const char *path;
    const char *query;
    const char *ref;
    const char *canon_url;
};

TEST(Url, parse)
{
    UrlCase cases[] = {
// URL                                    IsValid scheme  username password host   port  path        query  ref    Canon URL
// -------------------------------------  ------- ------- -------- -------- ----   ----  ----        -----  -----  -----------------------------------
{ "http://user:pass@foo:21/bar;par?b#c",  true,   "http", "user",  "pass",  "foo", 21,   "/bar;par", "b",   "c",  "http://user:pass@foo:21/bar;par?b#c" },
{ "Http://user:pass@foo:21/bar;par?b#",   true,   "http", "user",  "pass",  "foo", 21,   "/bar;par", "b",   "",   "http://user:pass@foo:21/bar;par?b#" },  
{ "HTTP://user:pass@foo:21/bar;par?b",    true,   "http", "user",  "pass",  "foo", 21,   "/bar;par", "b",   NULL, "http://user:pass@foo:21/bar;par?b" },  
{ "http://user:pass@foo:21/bar;par?",     true,   "http", "user",  "pass",  "foo", 21,   "/bar;par", "",    NULL, "http://user:pass@foo:21/bar;par?" },  
{ "http://user:pass@foo:21/bar;par?#c",   true,   "http", "user",  "pass",  "foo", 21,   "/bar;par", "",    "c",  "http://user:pass@foo:21/bar;par?#c" },  
{ "http://user:pass@foo:21/bar;par#c",    true,   "http", "user",  "pass",  "foo", 21,   "/bar;par", NULL,  "c",  "http://user:pass@foo:21/bar;par#c" },  
{ "http://user:pass@foo:21/bar;par",      true,   "http", "user",  "pass",  "foo", 21,   "/bar;par", NULL,  NULL, "http://user:pass@foo:21/bar;par" },  
{ "http://user:pass@foo:21/bar;par",      true,   "http", "user",  "pass",  "foo", 21,   "/bar;par", NULL,  NULL, "http://user:pass@foo:21/bar;par" },  
{ "http://user:pass@foo:21/",             true,   "http", "user",  "pass",  "foo", 21,   "/",        NULL,  NULL, "http://user:pass@foo:21/" },  
{ "http://user:pass@foo:21",              true,   "http", "user",  "pass",  "foo", 21,   "/",        NULL,  NULL, "http://user:pass@foo:21/" },  
{ "http://user:pass@foo:21/?b",           true,   "http", "user",  "pass",  "foo", 21,   "/",        "b",   NULL, "http://user:pass@foo:21/?b" },  
{ "http://user:pass@foo:21/?",            true,   "http", "user",  "pass",  "foo", 21,   "/",        "",    NULL, "http://user:pass@foo:21/?" },  
{ "http://user:pass@foo:21/?#",           true,   "http", "user",  "pass",  "foo", 21,   "/",        "",    "",   "http://user:pass@foo:21/?#" },  
{ "http://user:pass@foo:21/#",            true,   "http", "user",  "pass",  "foo", 21,   "/",        NULL,  "",   "http://user:pass@foo:21/#" },  
{ "http://user:@foo:21/",                 true,   "http", "user",  "",      "foo", 21,   "/",        NULL,  NULL, "http://user:@foo:21/" },  
{ "http://user@foo:21/",                  true,   "http", "user",  NULL,    "foo", 21,   "/",        NULL,  NULL, "http://user@foo:21/" },  
{ "http://user::p@foo:21/",               true,   "http", "user",  ":p",    "foo", 21,   "/",        NULL,  NULL, "http://user::p@foo:21/" },  
{ "http://user::p@@foo:21/",              true,   "http", "user",  ":p@",   "foo", 21,   "/",        NULL,  NULL, "http://user::p@@foo:21/" },  
{ "http://user::p@@:foo:21/",             true,   "http", "user",  ":p@",   ":foo",21,   "/",        NULL,  NULL, "http://user::p@@:foo:21/" },  
{ "http://user::p@@:foo:21//",            true,   "http", "user",  ":p@",   ":foo",21,   "/",        NULL,  NULL, "http://user::p@@:foo:21/" },  
{ "http:user::p@@:foo:21//??##",          true,   "http", "user",  ":p@",   ":foo",21,   "/",        "?",   "#",  "http://user::p@@:foo:21/??##" },  
{ "http:user::p@@:foo:21/?/??##",         true,   "http", "user",  ":p@",   ":foo",21,   "/",        "/??", "#",  "http://user::p@@:foo:21/?/??##" },  
{ "http://@foo:21/",                      true,   "http", "",      NULL,    "foo", 21,   "/",        NULL,  NULL, "http://@foo:21/" },  
{ "http://:@foo:21/",                     true,   "http", "",      "",      "foo", 21,   "/",        NULL,  NULL, "http://:@foo:21/" },  
{ "http://foo:21/",                       true,   "http", NULL,    NULL,    "foo", 21,   "/",        NULL,  NULL, "http://foo:21/" },  
{ "http://foo:/",                         true,   "http", NULL,    NULL,    "foo", -1,   "/",        NULL,  NULL, "http://foo/" },  
{ "http://foo/",                          true,   "http", NULL,    NULL,    "foo", -1,   "/",        NULL,  NULL, "http://foo/" },  
{ "http:///",                             false,  "http", NULL,    NULL,    NULL,  -1,   NULL,       NULL,  NULL, "http:" },  
{ ":///foo/",                             false,  "",     NULL,    NULL,    "foo", -1,   "/",        NULL,  NULL, "://foo/" },  
{ "///foo/",                              false,  NULL,   NULL,    NULL,    "foo", -1,   "/",        NULL,  NULL, "foo/" },  
{ "foo/",                                 false,  NULL,   NULL,    NULL,    "foo", -1,   "/",        NULL,  NULL, "foo/" },
// Creative URLs missing key elements
{ "",                                     false,  NULL,   NULL,    NULL,    NULL,  -1,   NULL,       NULL,  NULL, "" },
{ "   \t",                                false,  NULL,   NULL,    NULL,    NULL,  -1,   NULL,       NULL,  NULL, "" },
{ ":foo.com/",                            false,  "",     NULL,    NULL,    "foo.com",-1,"/",        NULL,  NULL, "://foo.com/" },
{ ":",                                    false,  "",     NULL,    NULL,    NULL,  -1,   NULL,       NULL,  NULL, ":" },
{ ":a",                                   false,  "",     NULL,    NULL,    "a",   -1,   "/",        NULL,  NULL, "://a/" },
{ ":/",                                   false,  "",     NULL,    NULL,    NULL,  -1,   NULL,       NULL,  NULL, ":" },
{ ":#",                                   false,  "",     NULL,    NULL,    NULL,  -1,   "/",        NULL,  "",   ":/#" },
{ "#",                                    false,  NULL,   NULL,    NULL,    NULL,  -1,   "/",        NULL,  "",   "/#" },
{ "#/",                                   false,  NULL,   NULL,    NULL,    NULL,  -1,   "/",        NULL,  "/",  "/#/" },
{ "#\\",                                  false,  NULL,   NULL,    NULL,    NULL,  -1,   "/",        NULL,  "\\", "/#\\" },
{ "#;?",                                  false,  NULL,   NULL,    NULL,    NULL,  -1,   "/",        NULL,  ";?", "/#;?" },
{ "?",                                    false,  NULL,   NULL,    NULL,    NULL,  -1,   "/",        "",    NULL, "/?" },
{ "/",                                    false,  NULL,   NULL,    NULL,    NULL,  -1,   NULL,       NULL,  NULL, "" },
{ ":23",                                  false,  "",     NULL,    NULL,    "23",  -1,   "/",        NULL,  NULL, "://23/" }, 
{ "/:23",                                 true,   "/",    NULL,    NULL,    "23",  -1,   "/",        NULL,  NULL, "/://23/" },
{ "//",                                   false,  NULL,   NULL,    NULL,    NULL,  -1,   NULL,       NULL,  NULL, "" },
{ "::",                                   false,  "",     NULL,    NULL,    NULL,  -1,   "/",        NULL,  NULL, ":///" },
{ "::23",                                 false,  "",     NULL,    NULL,    NULL,  23,   "/",        NULL,  NULL, "://:23/" },
{ "foo://",                               false,  "foo",  NULL,    NULL,    NULL,  -1,   NULL,       NULL,  NULL, "foo:" },

// Username/passwords
{ "http://a:b@c:29/d",                    true,   "http", "a",     "b",     "c",   29,   "/d",       NULL,  NULL, "http://a:b@c:29/d" },
{ "http::@c:29",                          true,   "http", "",      "",      "c",   29,   "/",        NULL,  NULL, "http://:@c:29/" },
{ "http://&a:foo(b]c@d:2/",               true,   "http", "&a",    "foo(b]c","d",  2,    "/",        NULL,  NULL, "http://&a:foo(b]c@d:2/" },
{ "http://::@c@d:2",                      true,   "http", "",      ":@c",   "d",   2,    "/",        NULL,  NULL, "http://::@c@d:2/" },
{ "http://foo.com:b@d/",                  true,   "http", "foo.com","b",    "d",   -1,   "/",        NULL,  NULL, "http://foo.com:b@d/" },

// Use the first question mark for the query and the ref.
{ "http://foo/path;a??e#f#g",             true,   "http", NULL,    NULL,    "foo", -1,   "/path;a",  "?e",  "f#g", "http://foo/path;a??e#f#g" },
{ "http://foo/abcd?efgh?ijkl",            true,   "http", NULL,    NULL,    "foo", -1,   "/abcd",    "efgh?ijkl", NULL, "http://foo/abcd?efgh?ijkl" },
{ "http://foo/abcd#foo?bar",              true,   "http", NULL,    NULL,    "foo", -1,   "/abcd",    NULL, "foo?bar", "http://foo/abcd#foo?bar" },

// IPv6, check also interesting uses of colons.
{ "[61:24:74]:98",                        true,   "[61",  NULL,    NULL,    "24:74]",98,  "/",       NULL,   NULL, "[61://24:74]:98/" },
{ "http://[61:27]:98",                    true,   "http", NULL,    NULL,    "[61:27]",98, "/",       NULL,   NULL, "http://[61:27]:98/" },
{ "http:[61:27]/:foo",                    true,   "http", NULL,    NULL,    "[61:27]",-1, "/:foo",   NULL,   NULL, "http://[61:27]/:foo" },
{ "http://[1::2]:3:4",                    true,   "http", NULL,    NULL,    "[1::2]:3",4, "/",       NULL,   NULL, "http://[1::2]:3:4/" },
{ "http://2001::1",                       true,   "http", NULL,    NULL,    "2001:",1,    "/",       NULL,   NULL, "http://2001::1/" },
{ "http://[2001::1",                      true,   "http", NULL,    NULL,    "[2001::1",-1,"/",       NULL,   NULL, "http://[2001::1/" },
{ "http://2001::1]",                      true,   "http", NULL,    NULL,    "2001::1]",-1,"/",       NULL,   NULL, "http://2001::1]/" },
{ "http://[[::]]",                        true,   "http", NULL,    NULL,    "[[::]]",-1,  "/",       NULL,   NULL, "http://[[::]]/" },

// Path canonicalization.
{ "http://foo.com///./a/b/..//c/d/../../e",true,  "http", NULL,    NULL,    "foo.com",-1, "/a/e",    NULL,   NULL, "http://foo.com/a/e" },
{ "http://foo.com///./../a/../../c/d/../e",true,  "http", NULL,    NULL,    "foo.com",-1, "/c/e",   NULL,   NULL, "http://foo.com/c/e"},
{ "http://foo.com///./../a/../../c/d/.../e",true,  "http", NULL,    NULL,    "foo.com",-1, "/c/d/.../e",NULL, NULL, "http://foo.com/c/d/.../e"},
};

    for (size_t i = 0; i < NMEM(cases); i++) {
        wsd::Url url(cases[i].input);

        EXPECT_EQ(cases[i].is_valid, url.isValid()) << "url=" << cases[i].input;
        if (cases[i].scheme)
            EXPECT_EQ(cases[i].scheme, url.getScheme());
        else
            EXPECT_FALSE(url.hasScheme()) << "url=" << cases[i].input;
        if (cases[i].username)
            EXPECT_EQ(cases[i].username, url.getUsername()) << "url=" << cases[i].input;
        else
            EXPECT_FALSE(url.hasUsername()) << "url=" << cases[i].input;
        if (cases[i].password)
            EXPECT_EQ(cases[i].password, url.getPassword()) << "url=" << cases[i].input;
        else
            EXPECT_FALSE(url.hasPassword()) << "url=" << cases[i].input;
        if (cases[i].host)
            EXPECT_EQ(cases[i].host, url.getHost()) << "url=" << cases[i].input;
        else
            EXPECT_FALSE(url.hasHost()) << "url=" << cases[i].input;
        if (cases[i].port)
            EXPECT_EQ(cases[i].port, url.getIntPort()) << "url=" << cases[i].input;
        else
            EXPECT_FALSE(url.hasPort()) << "url=" << cases[i].input;
        if (cases[i].path)
            EXPECT_EQ(cases[i].path, url.getPath()) << "url=" << cases[i].input;
        else
            EXPECT_FALSE(url.hasPath()) << "url=" << cases[i].input;
        if (cases[i].query)
            EXPECT_EQ(cases[i].query, url.getQuery()) << "url=" << cases[i].input;
        else
            EXPECT_FALSE(url.hasQuery()) << "url=" << cases[i].input;
        if (cases[i].ref)
            EXPECT_EQ(cases[i].ref, url.getRef()) << "url=" << cases[i].input;
        else
            EXPECT_FALSE(url.hasRef()) << "url=" << cases[i].input;
        EXPECT_EQ(cases[i].canon_url, url.getCanonUrl());
    }
}
