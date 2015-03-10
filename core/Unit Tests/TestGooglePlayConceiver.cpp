//
//  TestGooglePlayConceiver.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/27/13.
//
//

#include "tut/tut.hpp"
#include "GooglePlayConceiver.h"
#include "json.h"
#include "GearUtility.h"
#include "GooglePlayDirectSong.h"
#include "MockSession.h"


using namespace Gear;

// first songs, then albums, then artists
static const char testSearchReply[] = "\
[[0]\
,[[[\"T4h7vwg2dj4jx4pth5o74urbcmm\",\"Skinny Love\",\"//lh3.googleusercontent.com/1FvlDmRaq3BHUT7yJTOvx8PzHiEC6bv6Qjqj1jMWGoJnm850SZAGedHcJe0PGcG4uMFcvd4Pqw\",\"Birdy\",\"Birdy (Deluxe Version)\",\"Birdy\",\"skinny love\",\"birdy\",\"birdy (deluxe version)\",\"birdy\",,,,203000,2,,1,,,,,,0,-1,0,0,,\"T4h7vwg2dj4jx4pth5o74urbcmm\",\"T4h7vwg2dj4jx4pth5o74urbcmm\",5,,,\"B6dm7e5ucnmov6xd53mkutnmjjq\",\"A4i6ksl5kmnvfjap22zzdsjjyvi\",,,,,2,[]\
]\
,[\"T3yiugpkex6p7jmlyymcmvzzjbm\",\"Skinny Love\",\"//lh4.googleusercontent.com/yzO-wGxGgxOBSGhhWboYrJ0SHuVEP2yvnoPyta8WaHYwVRLg-gve4ZPgqse1IeA4ThSU8pib\",\"Bon Iver\",\"For Emma, Forever Ago\",\"Bon Iver\",\"skinny love\",\"bon iver\",\"for emma, forever ago\",\"bon iver\",,,,238000,3,,1,,,,,,0,-1,0,0,,\"T3yiugpkex6p7jmlyymcmvzzjbm\",\"T3yiugpkex6p7jmlyymcmvzzjbm\",5,,,\"Bayvujvcnydrhtz3beanjlz6fo4\",\"Almf54cg3d2djde3cbziyla3uri\",,,,,2,[]\
]\
,[\"Ttqfpiy7beapdab3d7mjpc6gufi\",\"Skinny Genes\",\"//lh3.googleusercontent.com/ujve1a45_LXG4-dgIHenpNvlNaANX-lxyAAXAkQXU-xvcB7wpq0bBKNqO8o32nYFpRsC496O\",\"Eliza Doolittle\",\"Eliza Doolittle\",\"Eliza Doolittle\",\"skinny genes\",\"eliza doolittle\",\"eliza doolittle\",\"eliza doolittle\",,,,185000,4,,1,,,,,,0,-1,0,0,,\"Ttqfpiy7beapdab3d7mjpc6gufi\",\"Ttqfpiy7beapdab3d7mjpc6gufi\",5,,,\"Bfc7iup7vbqfgusjjghppdznooq\",\"Axjuloyj3vouo2afxc3o7y2woti\",,,,,2,[]\
]\
,[\"Toxupnqy53c5hgpi55tcguymcqq\",\"Skinny\",\"//lh3.googleusercontent.com/NsqKKxXSv5VN-03Ki9HUUf7vlaXuagy19L4-NMxg646qYqNDP8_tpQH-KjjqtT-j2TaWAPOryA\",\"Filter\",\"Title Of Record\",\"Filter\",\"skinny\",\"filter\",\"title of record\",\"filter\",,,,343000,7,,1,,,,,,0,-1,0,0,,\"Toxupnqy53c5hgpi55tcguymcqq\",\"Toxupnqy53c5hgpi55tcguymcqq\",5,,,\"Bmvvwigj4qpfb2utpfjfjqko6be\",\"Amcbyweeuu3dw7al3g4lw4qbvyy\",,,,,2,[]\
]\
,[\"Tkzgtkknpinxnmizfqqcqsbfrta\",\"The Skinny\",\"//lh3.googleusercontent.com/smtP9Sako_k3oJKWyHwW5IPzAfJbw9srVWBe535ABgl6G9c-e3Ymp7XpaeaL2nfOTsVqSe1GMQ\",\"Atmosphere\",\"When Life Gives You Lemons, You Paint That Shit Gold\",\"Atmosphere\",\"the skinny\",\"atmosphere\",\"when life gives you lemons, you paint that shit gold\",\"atmosphere\",,,,216000,3,,1,,,,,,0,-1,0,0,,\"Tkzgtkknpinxnmizfqqcqsbfrta\",\"Tkzgtkknpinxnmizfqqcqsbfrta\",5,,,\"Bme4ztw5cb6zppbpgdhvwbjsyme\",\"Akaobzmkffroqhbnjuzyyx6hy4i\",,,,,1,[]\
]\
,[\"T24pt64xgnedgzvovtal6dy3gba\",\"Skinny Dippin'\",\"//lh5.googleusercontent.com/qUU2luIA6nAKmgiZkoQHSZhe5gCvbbjOttq0skVLirgiWiXn6nZQUvsrk52M91UdnCmTypRk45g\",\"Edens Edge\",\"Edens Edge\",\"Edens Edge\",\"skinny dippin'\",\"edens edge\",\"edens edge\",\"edens edge\",,,,223000,3,,1,,2012,,,,0,-1,0,0,,\"T24pt64xgnedgzvovtal6dy3gba\",\"T24pt64xgnedgzvovtal6dy3gba\",5,,,\"Blyf6pnnuwkfin7d63gz737z7wq\",\"Aa47h57p7itgwivuvmhgpyarbwm\",,,,,2,[]\
]\
,[\"Tbadez6yvf7cwqeksj7aqxhcp2a\",\"Skinny Love (Live)\",\"//lh4.googleusercontent.com/eC6DK2MSDr4-0JkDeQ7nPxmHKQOr_DpYkdFAIvz0QSC8JblCCz6mmjIj57y_wbvYY07LDswtfw\",\"Birdy\",\"Live In London\",\"Birdy\",\"skinny love (live)\",\"birdy\",\"live in london\",\"birdy\",,,,246000,8,,1,,2011,,,,0,-1,0,0,,\"Tbadez6yvf7cwqeksj7aqxhcp2a\",\"Tbadez6yvf7cwqeksj7aqxhcp2a\",5,,,\"Bbl2nd345iyb4tjjwynwprbkwk4\",\"A4i6ksl5kmnvfjap22zzdsjjyvi\",,,,,2,[]\
]\
,[\"Tcxw2vq4s7csnb75jriasoo5xym\",\"Skinny\",\"//lh3.googleusercontent.com/Ik76K6BxO0DnxcHzeevTkwPCp_N_gZnV5wRIlFUe62RxG7791BDpDgcexg3TG5b6c5g5utCZaA\",\"The Dollyrots\",\"Eat My Heart Out (Plus B-Sides)\",\"The Dollyrots\",\"skinny\",\"the dollyrots\",\"eat my heart out (plus b-sides)\",\"the dollyrots\",,,,105000,11,,1,,2004,,,,0,-1,0,0,,\"Tcxw2vq4s7csnb75jriasoo5xym\",\"Tcxw2vq4s7csnb75jriasoo5xym\",5,,,\"Bfhrej7elz2oz7lerym6d6i7rji\",\"Albt6epv5fxl3jj3byyjfqrazha\",,,,,2,[]\
]\
,[\"Tqwlnqo3lvrnirys5jcpiukmry4\",\"Skinny Papa\",\"//lh5.googleusercontent.com/uTyi3fJv-Z11dw7ig8-YwxREQ2GwRdhS7w4CqgGF0E816ukGQshaRWwRuvnxb9GVwdBewy33\",\"Willie Colon, Hector Lavoe\",\"El Malo\",\"Willie Colon\",\"skinny papa\",\"willie colon, hector lavoe\",\"el malo\",\"willie colon\",,,,245000,6,,1,,2012,,,,0,-1,0,0,,\"Tqwlnqo3lvrnirys5jcpiukmry4\",\"Tqwlnqo3lvrnirys5jcpiukmry4\",5,,,\"By5eyx7skniwd6jqg2jjehpnbfi\",\"Auxse2imu6edmnsrrjacoyeqewi\",,,,,2,[]\
]\
,[\"Tgm2pgileh7vsk4uzqam4mwmvfq\",\"Skinny Boy\",\"//lh4.googleusercontent.com/llIib4ycL_LuSzuNh3gLSgBc4FxZyR_p3KTRNLwYFnR0By5l3EEx_g_WJ32IrtUGUHKC0voyJUc\",\"Chicago\",\"Chicago VII (Expanded and Remastered)\",\"Chicago\",\"skinny boy\",\"chicago\",\"chicago vii (expanded and remastered)\",\"chicago\",,,,312000,15,,1,,,,,,0,-1,0,0,,\"Tgm2pgileh7vsk4uzqam4mwmvfq\",\"Tgm2pgileh7vsk4uzqam4mwmvfq\",5,,,\"Bafp56kf7gtp45ye462baf23hcy\",\"A72tjbk54ifrcji4732rug5l5je\",,,,,2,[]\
]\
]\
,[[,\"Bites\",\"Skinny Puppy\",\"http://lh5.ggpht.com/6OSuFftdiXpoj7RFQgDnmECJuppYd9mvibhkJ-4IPRLtdRYnl_CTC1dfkvgOIr6kRRDkUEtTsA\",0,0,[]\
,\"Bkhfyxmf3ammlar5yqqqlublwai\",[]\
,1985,\"A7gkcirwl4lpnnfgmti3unznhgq\",[]\
,,2]\
,[,\"Too Dark Park\",\"Skinny Puppy\",\"http://lh6.ggpht.com/x63Q9wpQPMjjh7xHH3i2dkUaOMm_6D3fPrwkiiO9UJLuTc_5CqFVaMkMGtRDXbIQGzsz3XKG\",0,0,[]\
,\"By5n6twtgnlepqcx4amdyau6nc4\",[]\
,1990,\"A7gkcirwl4lpnnfgmti3unznhgq\",[]\
,,2]\
,[,\"BRAP\",\"Skinny Puppy\",\"http://lh5.ggpht.com/_1VOdjDtiEchGfqjfC0bq3FKe2ne3aapgILAoelfN5CSDq9pTNHyj4M7TRQZeJ2XWsDAnRvvtQ\",0,0,[]\
,\"B2xjebfzthtu4vwj4bgi34a7vsm\",[]\
,1996,\"A7gkcirwl4lpnnfgmti3unznhgq\",[]\
,,2]\
,[,\"Last Rights\",\"Skinny Puppy\",\"http://lh5.ggpht.com/6-iidB4Nm1DabL8lCGCNmqQ3mKwQrIB4Cd5Qf9ZH4dbRrajdZzIPDPljdN09k2nUPwnMIg5E\",0,0,[]\
,\"Brm2npcnenm6p2kxkugwo6ftm3m\",[]\
,1992,\"A7gkcirwl4lpnnfgmti3unznhgq\",[]\
,,2]\
,[,\"Bootlegged, Broke and in Solvent Seas\",\"Skinny Puppy\",\"http://lh3.ggpht.com/4yCDui2GrOmJg7zaDF5P-4VF88WxABo7yYr5cmyYd9zXMJJ1dQlPCdhLk9rPEAAb0AvYSpuNPt0\",0,0,[]\
,\"Bj6gf3waetywmqfiiilgoh3yrze\",[]\
,2012,\"A7gkcirwl4lpnnfgmti3unznhgq\",[]\
,,2]\
,[,\"The Process\",\"Skinny Puppy\",\"http://lh5.ggpht.com/CmIQunf8hArrm7a_fyrzUxoSeVBSJ7Hid8rv_luPtiQvDCqFu3jR6Ih3oDnKdkzjY6a4powMNg\",0,0,[]\
,\"Bjssc7k5xcv3sqerspsa4fcjz3e\",[]\
,2012,\"A7gkcirwl4lpnnfgmti3unznhgq\",[]\
,,2]\
,[,\"Taller\",\"Skinny\",\"http://lh3.ggpht.com/Fhyv8Hlo4KB0KmfIxpAOQ5M60JlfFszRS42Rzdwqbx5-4HVyyq2az_tsSHJLft7_XLfhp8IaJQ\",0,0,[]\
,\"B335u7t3xviwcml3wkgawdqto7e\",[]\
,2001,\"Awjxdz7jof7p6lbpgfosegshwqu\",[]\
,,2]\
,[,\"The Singles Collect\",\"Skinny Puppy\",\"http://lh6.ggpht.com/J4DtzOol4KDMRTZWFhKazZfrp8DBNkulDyFRXaBmlVvwUkra9bq43uS8sWqYqv5G3OIzWXflclU\",0,0,[]\
,\"B5dmecvc4j2eo5hcstfa64b663u\",[]\
,1999,\"A7gkcirwl4lpnnfgmti3unznhgq\",[]\
,,2]\
,[,\"VIVI Sect VI\",\"Skinny Puppy\",\"http://lh4.ggpht.com/Ynxmn74-Qe8i1nYYSJzNgQjXr_dkv1Yc5gJ-qGlrAbsYUra0PFPYShTKj6a5w48pQ6ssNZFUqw\",0,0,[]\
,\"Bchgqxqnmiym4zazccl5gkpvcru\",[]\
,1988,\"A7gkcirwl4lpnnfgmti3unznhgq\",[]\
,,2]\
,[,\"Remission\",\"Skinny Puppy\",\"http://lh4.ggpht.com/HyZ6m3m4kho1l_7d1OCgh_a0bA49OmUThnIlBSKTbThrHLm9WfTeJNuQ8JPXUZi4YIOSsFF9CA\",0,0,[]\
,\"Bcdzec35pollgegi63kgdfvfluy\",[]\
,1984,\"A7gkcirwl4lpnnfgmti3unznhgq\",[]\
,,2]\
]\
,[[,\"A5j4lalwikr2mzafylumtfjxsi4\",\"Skinny\",0,[]\
,\"\",,,[]\
,[]\
,,[]\
]\
,[,\"A7gkcirwl4lpnnfgmti3unznhgq\",\"Skinny Puppy\",0,[]\
,\"http://lh6.ggpht.com/3un1qrUay_Euim9DPaHwtldYZf7pTpomwUkQHQCQHTdFldJ87vnp0Pcyzxf25H1jwsVRdd0AQw\",,,[]\
,[]\
,,[]\
]\
,[,\"Akiti2y6a7vv7d5no6umlcf3pnu\",\"Kingpin Skinny Pimp\",0,[]\
,\"http://lh4.ggpht.com/nlRsMmdfk1-EKL6A-f-C7R67YqqUrmezdp8niddXnS_o-xslfJ9i1rDnBMoLjTjkPTT5njNKKt8\",,,[]\
,[]\
,,[]\
]\
,[,\"A65m7gxnuarcrko7ualymimp2iq\",\"The Skinny\",0,[]\
,\"http://lh5.ggpht.com/vGQOGTKqCZYCxNy4DAyKDimfwD1agH7du_WstTjFos9ocXyraAxk9qNhnee1asxsi7VEDebmcQ\",,,[]\
,[]\
,,[]\
]\
,[,\"Af7chb4iivlmgsfgfa5vmcjywaq\",\"Skinny\",0,[]\
,\"http://lh6.ggpht.com/6pqLTKfoDQdJY-v3HwCveP5tW0M0l_jx_C9IekZwWK3u0SSaGefkLWs98oUlr-0tuuRP1bWF_g\",,,[]\
,[]\
,,[]\
]\
,[,\"Aj4dpm5ah5f4i6fxo5qxttc7o7m\",\"Skinny\",0,[]\
,\"http://lh5.ggpht.com/oDHQo0Kiz1e6XTiDX6FNZ2pQUolEN_JYyFPn4sRyHnlWa6WZOaOkC9LIc-9kC2q2U1a42kT1FQ\",,,[]\
,[]\
,,[]\
]\
,[,\"Amzfacrwpuzlsyefn2cvmcvf5yu\",\"Skinny Lister\",0,[]\
,\"http://lh3.ggpht.com/LiWxbFAtMqss3rc55vZUltnJWT-L-owixhsilwAycqOBTmGpbxRfiud8xL5VcoXbdVLBG7w9Qg\",,,[]\
,[]\
,,[]\
]\
,[,\"Aksodaifojb7lgf6w2gs66w6d2e\",\"Skinny\",0,[]\
,\"http://lh3.ggpht.com/UvJN_dBJFhMHt1_szn5c2rCSK5h--FGplQF174kwG8GnfovlXjA5htcHVkA-TbVEiBuoASxRJw\",,,[]\
,[]\
,,[]\
]\
,[,\"Atadjzrlsk6mowlgwfdu5eo5ue4\",\"Skinny Banton\",0,[]\
,\"http://lh4.ggpht.com/AOA58z5EzLxlrw_XI8urwDzZ-RopMMwIjaDYBOnO5fDdHo3DuT4WvsdJHC_a2ZVxjzQnNl81\",,,[]\
,[]\
,,[]\
]\
,[,\"A34t7l7t6ossiqpidzur4ba3wji\",\"Rollerskate Skinny\",0,[]\
,\"http://lh3.ggpht.com/3rUNovH_o2sb7Zk45x6d0hHN_HmgdGhlxo5WdBcggRTLNeRJHGBv1Rxn8-bopNHIUjwTfQQ4rw\",,,[]\
,[]\
,,[]\
]\
]\
]\
]";

static const char testSearchForConcreteAlbum[] = "\
[[0]\
,[[[\"Tjlyldozsfdpaegqzwnuvbdyumi\",\"Home\",\"//lh6.googleusercontent.com/acX8LT5bkXOBgWmYdBTFO6SdxevZ_B3d3YpcFq_FugGx4nZ-WOymabid2oGB8pQ7ahPsyhqgaw\",\"Nosaj Thing\",\"Home\",\"Nosaj Thing\",\"home\",\"nosaj thing\",\"home\",\"nosaj thing\",,,,159000,1,,1,,2013,,,,0,-1,0,0,,\"Tjlyldozsfdpaegqzwnuvbdyumi\",\"Tjlyldozsfdpaegqzwnuvbdyumi\",5,,,\"Baitatm2ksm6bgnpiji26t4rh2m\",\"Axkwx5gjjkhiwbyebrr5ggigd74\",,,,,2,[]\
,\"AE9vGKqVZMICRCBOHpSSQtKo379zKKR6hC4gWMAIhCuoGdJMyH8tdh5IEfqbHJ0REg6hPIvIrlrIgrpaL9tQ9RnK186bANzqftBXh_OcUFY9w1RKjRZ9Gwk\\u003d\"]\
]\
,[]\
,[]\
,[,[,\"Home\",\"Nosaj Thing\",\"http://lh6.ggpht.com/acX8LT5bkXOBgWmYdBTFO6SdxevZ_B3d3YpcFq_FugGx4nZ-WOymabid2oGB8pQ7ahPsyhqgaw\",0,0,[]\
,\"Baitatm2ksm6bgnpiji26t4rh2m\",[]\
,2013,\"Axkwx5gjjkhiwbyebrr5ggigd74\",[]\
,,2]\
]\
,,[,\"Home\",\"Nosaj Thing\",\"http://lh6.ggpht.com/acX8LT5bkXOBgWmYdBTFO6SdxevZ_B3d3YpcFq_FugGx4nZ-WOymabid2oGB8pQ7ahPsyhqgaw\",0,0,[]\
,\"Baitatm2ksm6bgnpiji26t4rh2m\",[]\
,2013,\"Axkwx5gjjkhiwbyebrr5ggigd74\",[]\
,,2]\
]\
]\
";

static const char testSearchForConcreteArtist[] = "\
[[0]\
,[[[\"Ti6zpwt3vpzipgj362xaery2l7q\",\"1685/Bach\",\"//lh3.googleusercontent.com/ZqgEmiML6ZErhBBGbwa04zKs_CDd3j4JmIVJtk0vAkZ1P52bqIge9fCK1LCwzcj4KtvDUhWeVw\",\"Nosaj Thing\",\"Drift\",\"Nosaj Thing\",\"1685/bach\",\"nosaj thing\",\"drift\",\"nosaj thing\",,,,168000,5,,1,,2009,,,,0,-1,0,0,,\"Ti6zpwt3vpzipgj362xaery2l7q\",\"Ti6zpwt3vpzipgj362xaery2l7q\",5,,,\"Bczpp5y4sg4vtcltlgo7gsmju3e\",\"Axkwx5gjjkhiwbyebrr5ggigd74\",,,,,2,[]\
,\"AE9vGKoJSgPm6Y0AmT_c1lB5BMO6OjB8mnHOWWChtvs_ssr6KYfGodyZFhM8mYoJFZiFuS4QPodz_3tiPaRVm_3PxWM0BpPtoTt_JScS2fgfu9NRrylpYmw\\u003d\"]\
,[\"Tbyslzk5lxqgazm35mc3lr3ixje\",\"Coat of Arms\",\"//lh3.googleusercontent.com/ZqgEmiML6ZErhBBGbwa04zKs_CDd3j4JmIVJtk0vAkZ1P52bqIge9fCK1LCwzcj4KtvDUhWeVw\",\"Nosaj Thing\",\"Drift\",\"Nosaj Thing\",\"coat of arms\",\"nosaj thing\",\"drift\",\"nosaj thing\",,,,208000,3,,1,,2009,,,,0,-1,0,0,,\"Tbyslzk5lxqgazm35mc3lr3ixje\",\"Tbyslzk5lxqgazm35mc3lr3ixje\",5,,,\"Bczpp5y4sg4vtcltlgo7gsmju3e\",\"Axkwx5gjjkhiwbyebrr5ggigd74\",,,,,2,[]\
,\"AE9vGKo1IZsEKfcYXxSO4UpJV8_TlBTEKgQlyg5XIO-xqZ54SH21PmTA1aLIkX38crPHDfc2OKPKKVGr1MX4tlvpse92kUKl49V8Kw0Z6hMzMpDDY3xHpYY\\u003d\"]\
,[\"Tktfeqhgx7jrcrkhqmfv3gyofdq\",\"Us\",\"//lh3.googleusercontent.com/ZqgEmiML6ZErhBBGbwa04zKs_CDd3j4JmIVJtk0vAkZ1P52bqIge9fCK1LCwzcj4KtvDUhWeVw\",\"Nosaj Thing\",\"Drift\",\"Nosaj Thing\",\"us\",\"nosaj thing\",\"drift\",\"nosaj thing\",,,,186000,10,,1,,2009,,,,0,-1,0,0,,\"Tktfeqhgx7jrcrkhqmfv3gyofdq\",\"Tktfeqhgx7jrcrkhqmfv3gyofdq\",5,,,\"Bczpp5y4sg4vtcltlgo7gsmju3e\",\"Axkwx5gjjkhiwbyebrr5ggigd74\",,,,,2,[]\
,\"AE9vGKpRAWNEHLcwigywEo68bfGmqffb4Jg9g2QS3d70cjzgIzIIVlquAbI0J1EsFTX5QtY6VWovwxRn08DQ1V35YfleT9evH_mbxuU7F7i6HEQa1Vvvus4\\u003d\"]\
,[\"Td2e3i7ywfqmfhlxx5cs3umf7gu\",\"Nightcrawler\",\"//lh3.googleusercontent.com/86vf_ZuRtuepyuNYJgqNEW03Wqon7FQCQxpWY-CMLuW2YBIN38mvkKBhX4QyEENQtZ4lMiZkLQ\",\"Nosaj Thing\",\"We Are The Works In Progress\",\"Various Artists\",\"nightcrawler\",\"nosaj thing\",\"we are the works in progress\",\"various artists\",,,,230000,4,,1,,,,,,0,-1,0,0,,\"Td2e3i7ywfqmfhlxx5cs3umf7gu\",\"Td2e3i7ywfqmfhlxx5cs3umf7gu\",5,,,\"Bo5pfxgru75ilskd5tx3odr27v4\",\"Axkwx5gjjkhiwbyebrr5ggigd74\",,,,,2,[]\
,\"AE9vGKqG8G6pE9sNzyU8DM-Aysoj8GxX3iD9NWj5TljqZagoVP8vnBuOeExdDO_HJzmQ-syePNewO5crBm8_hCHqYfeibN8j5nUSrimtU8dCHBigK1r7Ijo\\u003d\"]\
,[\"Ti3smtj3ozq265a6xmrltbje45m\",\"Eclipse/Blue (feat. Kazu Makino)\",\"//lh6.googleusercontent.com/acX8LT5bkXOBgWmYdBTFO6SdxevZ_B3d3YpcFq_FugGx4nZ-WOymabid2oGB8pQ7ahPsyhqgaw\",\"Nosaj Thing\",\"Home\",\"Nosaj Thing\",\"eclipse/blue (feat. kazu makino)\",\"nosaj thing\",\"home\",\"nosaj thing\",,,,269000,2,,1,,2013,,,,0,-1,0,0,,\"Ti3smtj3ozq265a6xmrltbje45m\",\"Ti3smtj3ozq265a6xmrltbje45m\",5,,,\"Baitatm2ksm6bgnpiji26t4rh2m\",\"Axkwx5gjjkhiwbyebrr5ggigd74\",,,,,2,[]\
,\"AE9vGKpptEeKMkoN5vdGEnmUT_TwjlHsdym6czZ89AoCPPykkLRLxWwVb1Jnx54uk_LCIasUcZ1tT70623-sQTdiqfhrBd58IBMwhwkrrsCpSydQWF1QWdw\\u003d\"]\
,[\"Tkusolodk5n35hmo7kuj5a2seni\",\"Fog (Jamie xx Remix)\",\"//lh3.googleusercontent.com/Gs8pP1wKDtWH-UisxhbNnmz4WOU0abfVTurIOFz_BkmTbS9aL3NdJYR0fSICyz91xjF78TphXQ\",\"Nosaj Thing\",\"Drift Remixed\",\"Nosaj Thing\",\"fog (jamie xx remix)\",\"nosaj thing\",\"drift remixed\",\"nosaj thing\",,,,280000,5,,1,,2009,,,,0,-1,0,0,,\"Tkusolodk5n35hmo7kuj5a2seni\",\"Tkusolodk5n35hmo7kuj5a2seni\",5,,,\"B5f2cdfwbpkgwylspz353yi3nxq\",\"Axkwx5gjjkhiwbyebrr5ggigd74\",,,,,2,[]\
,\"AE9vGKqTgl8iGmNqVwlSjBUt7Cdt-h71n6f-GPLvcxAX2zCnh5tDxlHIONnd9mu8NUtUB8awsobHoorbY8NfMcDg9UGI_L3n6CvfBhSJAZUsvZPS70ct_xk\\u003d\"]\
,[\"Tpsyamcmrvbq7jvxum2nrdnh6aa\",\"Fog\",\"//lh3.googleusercontent.com/ZqgEmiML6ZErhBBGbwa04zKs_CDd3j4JmIVJtk0vAkZ1P52bqIge9fCK1LCwzcj4KtvDUhWeVw\",\"Nosaj Thing\",\"Drift\",\"Nosaj Thing\",\"fog\",\"nosaj thing\",\"drift\",\"nosaj thing\",,,,222000,2,,1,,2009,,,,0,-1,0,0,,\"Tpsyamcmrvbq7jvxum2nrdnh6aa\",\"Tpsyamcmrvbq7jvxum2nrdnh6aa\",5,,,\"Bczpp5y4sg4vtcltlgo7gsmju3e\",\"Axkwx5gjjkhiwbyebrr5ggigd74\",,,,,2,[]\
,\"AE9vGKpqKJh--j3UnBaAsSeiR_KwHacKz_zkFnJFv8LdeM6OAI9j5Jt16g60Cgd-Q-bplAKuHk8-9l1VbN-Cgqd6DQ4BcgTAfic5SJydRuL5RIHCBBoxnls\\u003d\"]\
,[\"Tmijt3felswaauqq7msbvdgkr5q\",\"Try (feat. Toro y Moi)\",\"//lh6.googleusercontent.com/acX8LT5bkXOBgWmYdBTFO6SdxevZ_B3d3YpcFq_FugGx4nZ-WOymabid2oGB8pQ7ahPsyhqgaw\",\"Nosaj Thing\",\"Home\",\"Nosaj Thing\",\"try (feat. toro y moi)\",\"nosaj thing\",\"home\",\"nosaj thing\",,,,242000,9,,1,,2013,,,,0,-1,0,0,,\"Tmijt3felswaauqq7msbvdgkr5q\",\"Tmijt3felswaauqq7msbvdgkr5q\",5,,,\"Baitatm2ksm6bgnpiji26t4rh2m\",\"Axkwx5gjjkhiwbyebrr5ggigd74\",,,,,2,[]\
,\"AE9vGKrwlBlCNQe-wImY_JoCnSd4P9_IJxIsp1fjO5H4zNkIxehKWPwL7KymwPGM_z-ur1CRWIUGh-GiPugT-ROG41KL-dxy8eoqIm08Tv_KSoIwcdjZxHM\\u003d\"]\
,[\"Tnvxz42jkc67tfpdzbdver7ptta\",\"Heart Entire\",\"//lh3.googleusercontent.com/Rzzj-oazMornPEqLDhqMwMwrxGmA1vRZX93g8VoTY-vbSF76kkmPUUwxrbaZOugneJfP1YLS\",\"NOSAJ THING\",\"Daly City Sf Laptop \\u0026 Machine Battle 2007\",\"Various Artists\",\"heart entire\",\"nosaj thing\",\"daly city sf laptop \\u0026 machine battle 2007\",\"various artists\",,,,193000,6,,1,,,,,,0,-1,0,0,,\"Tnvxz42jkc67tfpdzbdver7ptta\",\"Tnvxz42jkc67tfpdzbdver7ptta\",5,,,\"Bnjcmqvl6nqconaybeqcce225fa\",\"Axkwx5gjjkhiwbyebrr5ggigd74\",,,,,3,[]\
,\"AE9vGKpdCKSqy_JnoJGy-vhrSKnN-zrxUg0DzsXA5Lf-HP1ckjhzJcGqQgaUZJiwpD-KTYtA39SoOWitrQ0-wDGAgvvHJlUCvqDHeD3YkZ_czT1cvB0yNak\\u003d\"]\
,[\"T5qvvc7ylwyai7eu5gp2lbskwbu\",\"Light #1\",\"//lh3.googleusercontent.com/ZqgEmiML6ZErhBBGbwa04zKs_CDd3j4JmIVJtk0vAkZ1P52bqIge9fCK1LCwzcj4KtvDUhWeVw\",\"Nosaj Thing\",\"Drift\",\"Nosaj Thing\",\"light #1\",\"nosaj thing\",\"drift\",\"nosaj thing\",,,,175000,7,,1,,2009,,,,0,-1,0,0,,\"T5qvvc7ylwyai7eu5gp2lbskwbu\",\"T5qvvc7ylwyai7eu5gp2lbskwbu\",5,,,\"Bczpp5y4sg4vtcltlgo7gsmju3e\",\"Axkwx5gjjkhiwbyebrr5ggigd74\",,,,,2,[]\
,\"AE9vGKogtcGwrcuBmQW01UuDDMl6HcO6HK8mK9EmByj1TOacwXNjlBm9swuqA9px2_CzQCNkBrqSYJXjTIIgbyIFpUJd3ed0WVEQQfrLxCksuJqFy8-P_uY\\u003d\"]\
]\
,[[,\"Drift\",\"Nosaj Thing\",\"http://lh3.ggpht.com/ZqgEmiML6ZErhBBGbwa04zKs_CDd3j4JmIVJtk0vAkZ1P52bqIge9fCK1LCwzcj4KtvDUhWeVw\",0,0,[]\
,\"Bczpp5y4sg4vtcltlgo7gsmju3e\",[]\
,2009,\"Axkwx5gjjkhiwbyebrr5ggigd74\",[]\
,,2]\
,[,\"Drift Remixed\",\"Nosaj Thing\",\"http://lh3.ggpht.com/Gs8pP1wKDtWH-UisxhbNnmz4WOU0abfVTurIOFz_BkmTbS9aL3NdJYR0fSICyz91xjF78TphXQ\",0,0,[]\
,\"B5f2cdfwbpkgwylspz353yi3nxq\",[]\
,2009,\"Axkwx5gjjkhiwbyebrr5ggigd74\",[]\
,,2]\
]\
,[]\
,[,,[,\"Axkwx5gjjkhiwbyebrr5ggigd74\",\"Nosaj Thing\",0,[]\
,\"http://lh6.ggpht.com/PnTs_ankHcVbf4q8X08MaIh1kNLHMlqjrMbPPRWDp5HK3RtV3W5K4zLHXoKGVgRM50J6U0XPLg\",,,[]\
,[]\
,,[]\
]\
]\
,,[,\"Home\",\"Nosaj Thing\",\"http://lh6.ggpht.com/acX8LT5bkXOBgWmYdBTFO6SdxevZ_B3d3YpcFq_FugGx4nZ-WOymabid2oGB8pQ7ahPsyhqgaw\",0,0,[]\
,\"Baitatm2ksm6bgnpiji26t4rh2m\",[]\
,2013,\"Axkwx5gjjkhiwbyebrr5ggigd74\",[]\
,,2]\
,[,\"Axkwx5gjjkhiwbyebrr5ggigd74\",\"Nosaj Thing\",0,[]\
,\"http://lh6.ggpht.com/PnTs_ankHcVbf4q8X08MaIh1kNLHMlqjrMbPPRWDp5HK3RtV3W5K4zLHXoKGVgRM50J6U0XPLg\",,,[]\
,[]\
,,[]\
]\
]\
]\
";

static const char testArtistFetch[] = "\
[[0]\
,[[[,\"Abrieyefqd7zyfsnd7ttway5qgy\",\"Goa Gil\",0,[[,\"Worldbridger\",\"Goa Gil\",\"http://lh3.ggpht.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",0,0,[]\
,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",[]\
,2007,\"Abrieyefqd7zyfsnd7ttway5qgy\",[]\
,,2]\
]\
,\"http://lh4.ggpht.com/_G068XZfsrEV5gZUHXZapnaFcWqsWvJqs6C88QmFchZCgF46wxMPJwRDYCZhksXyiP_aFnwQ2A\",,,[]\
,[[,\"Atngmlxlixottlthpcypidmxlu4\",\"Deadmau5\",0,[]\
,\"http://lh3.ggpht.com/REvYUg3fVpmAGQ-vSI7jPpoAbPuCREQcsHbranRdPi8QPbl6I9qwHAOhJd2CrgT4X1ysSDCOhA\",,,[]\
,[]\
,,[]\
]\
,[,\"Antiemo7xwnzofue2ge7hybvpt4\",\"Bassnectar\",0,[]\
,\"http://lh5.ggpht.com/hOl8iK19V3GsTY1Oplg3i8pqk36AuxJTUxpp-rxWw75nMkX_jhnLj-h--Mlx1uHausUI2Q1rXw\",,,[]\
,[]\
,,[]\
]\
,[,\"Adhiec2ed2lvaglrmt52f3xu3ie\",\"The Chemical Brothers\",0,[]\
,\"https://lh5.ggpht.com/uZcFeibx10B3WU4Ttz1ft8W4GKnY1aXoBds4GrklnvzVM8OTHYCpufc0eic9W1s5oYGWgaW6aw\",,,[]\
,[]\
,,[]\
]\
,[,\"Afp2gizke2x5gjmweeutvncznq4\",\"Infected Mushroom\",0,[]\
,\"http://lh6.ggpht.com/kaiBMYZ88bBDGUowdVjgFCNAx_v80-YzUKYWyd2nYylKPTu5IwerXJcMR4LAQgJYMS8wZ0jtsA\",,,[]\
,[]\
,,[]\
]\
,[,\"Aruxfsh75imhlfzbzferobgbghm\",\"Crystal Method\",0,[]\
,\"https://lh6.ggpht.com/x_vdrM0S9uQYyIwLZlVeTOdU-78wP8iNJQhdNXKPBrT7rH6wQB_PplM4wUBT5zQh_nEfDtNThQ\",,,[]\
,[]\
,,[]\
]\
,[,\"A2fq3wwz25xla7czwcrizooqyry\",\"Noisia\",0,[]\
,\"http://lh4.ggpht.com/sbKj4sHZvhL9orKHFNSO2sGun5TrKvRaHlVojPM7sYbn6-PDkXGD3vT87pGpUEF_eyr5X4mZ\",,,[]\
,[]\
,,[]\
]\
,[,\"Agzf4r7d54azste62qh6z2q7bcy\",\"Tiësto\",0,[]\
,\"http://lh5.ggpht.com/R1qwTrlseTXP666UKjtfBJ7I6pMWd_ltheb8WZ7GpsKNEB5kPkBKR83-Jtg7Pcmah2-t-G4v7w\",,,[]\
,[]\
,,[]\
]\
,[,\"At43tgdn2zrm3bhbp2uky5626ni\",\"Flying Lotus\",0,[]\
,\"http://lh6.ggpht.com/AHcNPIYDfw8rIMC-ef3QWW-AQ0Xsd2qfgTuIwMyqDMLAWgJ353YYvIW8LLELBgmLq8Y7kippVQ8\",,,[]\
,[]\
,,[]\
]\
,[,\"Ap6exarirw2ls3jjrhnf5p2lcvm\",\"Daft Punk\",0,[]\
,\"http://lh6.ggpht.com/o2uJ47AczwwIQFOMycSXpBbKSjbAvV_1wv3NHJweMnb0fV5iolbP7lGiUt_AguS_iXy07NRK0TI\",,,[]\
,[]\
,,[]\
]\
,[,\"Adbugxnsjn4gptoljajngyu5h3q\",\"Skream\",0,[]\
,\"http://lh5.ggpht.com/TQTr99ZsCcuxeMu04ACjHqtViydy05px8i2imb8M__g87XQ0W-Yj8df1xTpHH9kyAqzW6Z4o\",,,[]\
,[]\
,,[]\
]\
,[,\"Aq5xe3lznblgooedgsfkkkgn6e4\",\"Mr. Oizo\",0,[]\
,\"http://lh4.ggpht.com/fg0O29SH7gMx_Be_FDuI7DvukfxsZdQcfYG7-X1koMrf09Y1NT2lBdM70UxLxVJriP0EJJSV\",,,[]\
,[]\
,,[]\
]\
,[,\"Ao62kujeondku3c4pa5lumsnqd4\",\"DJ Icey\",0,[]\
,\"http://lh4.ggpht.com/kh0eA_fb6nTm67y-hUE9zK05XKS53YldPzwgEHRIFQ4C--exRRYmrQDFzZU-53Lc7CfQpWKy\",,,[]\
,[]\
,,[]\
]\
,[,\"A7ihjt5dn2627dcioikmbhlhjka\",\"Modeselektor, PVT\",0,[]\
,\"http://lh5.ggpht.com/rhFxpPcYgYGKF60RieL3jXdjc_9KG4bijv-IdPV5s7h3ohX03isqyL0s0SUMgmazvrzfFn8yl8s\",,,[]\
,[]\
,,[]\
]\
,[,\"Az6ci2oussoklhglvbszzvpvy2e\",\"Boys Noize\",0,[]\
,\"http://lh3.ggpht.com/4U1buz-Cm7akRzDB7NqTdYxIB-0hLpuHacwq7LrsFB38UpkHQGg3vHVcu5oYg7O0SBZPB5Ey\",,,[]\
,[]\
,,[]\
]\
,[,\"Ayoidtvwv3jchewjeahrw3vuw7i\",\"The Prodigy\",0,[]\
,\"https://lh5.ggpht.com/csIir7Hp3uZHT_cnWiiizOBCmVyzkJJXIOjPIRlhrwXeAXsvFNLQP6MjCKqJn4GtwDn8EU4oS6o\",,,[]\
,[]\
,,[]\
]\
,[,\"Arwpidedwzzivb36hmtcd7fsgc4\",\"Timo Maas, Santos\",0,[]\
,\"http://lh6.ggpht.com/Jjg6wVpwePP39QpXlCEcZSP6mnwqVcV40S91oUsSw-JEI6OV-VDZcIiKEeBixM0JCUnx2Qeo\",,,[]\
,[]\
,,[]\
]\
,[,\"Axs7ndoga46xcsggyj3mgtdtuiq\",\"Benga\",0,[]\
,\"http://lh4.ggpht.com/wDrLcHILthmroKFJyD12ZlF9VhXZuehUrnl5q8Krg1gAe-L-a8-PpR5TyP5CORybGmmGOEfIhA\",,,[]\
,[]\
,,[]\
]\
,[,\"Aha73h4qadkud7tda2bux5lvrk4\",\"Tiga\",0,[]\
,\"http://lh3.ggpht.com/GqC9MSH9DFqMEegIQ_nQ4imLQ5LKIERTbAmlnzh4u4CZPP1h2hIye_hi_iqJnTD8-t9J296OHw\",,,[]\
,[]\
,,[]\
]\
,[,\"Amnp4kbyhk55v3iz7c2cz262vm4\",\"Black Sun Empire\",0,[]\
,\"http://lh3.ggpht.com/S9y3lW1HRX6scZkv09Oem7CiM-h_mUVLxNd1Xphm-3O6KKHVwgTDIoaYkIRw5EkysgXz6PDc3g\",,,[]\
,[]\
,,[]\
]\
,[,\"Al2gwn5dkfmqs2w7d7lqbxdahm4\",\"Plump DJs\",0,[]\
,\"http://lh5.ggpht.com/kJNKuI_8DE8j94y3bZQhUr4EDoTipgmotVPnPyYk75z-feN0Gi79V9PvASTh0YHTAIrSJFtdgg\",,,[]\
,[]\
,,[]\
]\
]\
,\"If anyone's been in the trance scene longer than Goa Gil, it would be hard to name the person. Gil has been releasing albums since the early '90s, but his love of the region of Goa and its spiritual practices goes back to 1969. That was the year he left San Francisco, which was of course in the middle of its flower child heyday, to travel to the furthest reaches of India. Many people credit the dreadlocked DJ with creating the subgenre Goa trance. Some of the very first Goa Full Moon parties, which are famed among scenesters and travelers the world over, were organized by Gil -- and he continues to organize them even today. His constant companion on- and off-stage is Ariane, a fellow Gil trance pioneer and a renowned African Djembe drummer. As a musical team, Gil and Ariane go under the name the Nommos. \\n\\nGil is a DJ first, a remixer second. His releases are split between remixes of fellow trance artists and DJ sets from live shows. Many of the albums feature Gil remixing Kode IV, his labelmate on San Francisco record label Ceiba as well as its owner. The first release, Insane, featured him on only two tracks. Gil's first full-length CD, entitled Spiritual Trance, was released on the French label Fairway Records in 1995. Spiritual Trance II followed in 1996. Gil has also released albums on Deck Wizards (England), NMC/Avatar (Israel), Phonokol Records (Israel), and Sony India. He is probably most famous in Israel, which remains a hot spot for trance, and San Francisco, the place where he started his \\\"psychedelic journey\\\" more than 30 years ago. He continues to travel around the world, playing and occasionally throwing parties everywhere from the Marin redwood forests to the crowded cities of the Mediterranean to the Middle Eastern desert. His most recent release was Cyber Baba 2000 (Phonokol Records, August 2000).\\n\\nL. Katz, Rovi\",[[\"T2opkam5ui3dhr5d4nmgjngnsw4\",\"Cosmo vs Cerebral Factory - Reality Hackers\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"cosmo vs cerebral factory - reality hackers\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,400000,10,,1,,,,,,0,-1,0,0,,\"T2opkam5ui3dhr5d4nmgjngnsw4\",\"T2opkam5ui3dhr5d4nmgjngnsw4\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tut2an427pk2tnuarp5hxriwhwy\",\"Kulu - Virtual Revolution\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"kulu - virtual revolution\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,369000,9,,1,,,,,,0,-1,0,0,,\"Tut2an427pk2tnuarp5hxriwhwy\",\"Tut2an427pk2tnuarp5hxriwhwy\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tymeivb7uwt2fkzwc2azdolo3xe\",\"Intro - Aum\",\"//lh3.googleusercontent.com/VsMzbWLHd9PkMkqtSOiPDm7-_hGsMOtSw_9TBXwXIExFM7Mq8H8oJGxsUgkxj-PbA3sY_oQCmw\",\"Goa Gil\",\"Goa Gil / Worldbridger\",\"Various Artists\",\"intro - aum\",\"goa gil\",\"goa gil / worldbridger\",\"various artists\",,,,27000,1,,1,,,,,,0,-1,0,0,,\"Tymeivb7uwt2fkzwc2azdolo3xe\",\"Tymeivb7uwt2fkzwc2azdolo3xe\",5,,,\"Bysadhu6b3dj5kwuap4ccjxneje\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tbnvxo3tijzmvfuefzr4xvedwk4\",\"Horror Place -The Search For Truth\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"horror place -the search for truth\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,453000,1,,1,,,,,,0,-1,0,0,,\"Tbnvxo3tijzmvfuefzr4xvedwk4\",\"Tbnvxo3tijzmvfuefzr4xvedwk4\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tpzocrgtohekegwixnxozcujwh4\",\"Kin Dza Dza - 9 Energy Transformers\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"kin dza dza - 9 energy transformers\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,353000,5,,1,,,,,,0,-1,0,0,,\"Tpzocrgtohekegwixnxozcujwh4\",\"Tpzocrgtohekegwixnxozcujwh4\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Twvxj5tr6mtrwflsx2rdcwfzwfu\",\"Stranger - The Worldbridger\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"stranger - the worldbridger\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,312000,6,,1,,,,,,0,-1,0,0,,\"Twvxj5tr6mtrwflsx2rdcwfzwfu\",\"Twvxj5tr6mtrwflsx2rdcwfzwfu\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tga3aeodvc6b55her7gbxil5eie\",\"The Nommos - Between Worlds\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"the nommos - between worlds\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,413000,3,,1,,,,,,0,-1,0,0,,\"Tga3aeodvc6b55her7gbxil5eie\",\"Tga3aeodvc6b55her7gbxil5eie\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tah26tu5abvt34ax3rborkxmcnm\",\"Savage Scream - Shadow Walker\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"savage scream - shadow walker\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,418000,8,,1,,,,,,0,-1,0,0,,\"Tah26tu5abvt34ax3rborkxmcnm\",\"Tah26tu5abvt34ax3rborkxmcnm\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tdeayzk3ty3htxqakuixrygm6oa\",\"Outro - Tomorrows World\",\"//lh3.googleusercontent.com/VsMzbWLHd9PkMkqtSOiPDm7-_hGsMOtSw_9TBXwXIExFM7Mq8H8oJGxsUgkxj-PbA3sY_oQCmw\",\"Goa Gil\",\"Goa Gil / Worldbridger\",\"Various Artists\",\"outro - tomorrows world\",\"goa gil\",\"goa gil / worldbridger\",\"various artists\",,,,18000,13,,1,,,,,,0,-1,0,0,,\"Tdeayzk3ty3htxqakuixrygm6oa\",\"Tdeayzk3ty3htxqakuixrygm6oa\",5,,,\"Bysadhu6b3dj5kwuap4ccjxneje\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tajin3kv7ae5wtbirggpkotgkem\",\"Orestis - Contact With Peace\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"orestis - contact with peace\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,377000,2,,1,,,,,,0,-1,0,0,,\"Tajin3kv7ae5wtbirggpkotgkem\",\"Tajin3kv7ae5wtbirggpkotgkem\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tgzczfq7tupo4dogvde7k5bbf2a\",\"Sonik Scizzor - Nature's Roar\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"sonik scizzor - nature's roar\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,440000,11,,1,,,,,,0,-1,0,0,,\"Tgzczfq7tupo4dogvde7k5bbf2a\",\"Tgzczfq7tupo4dogvde7k5bbf2a\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tc7neuug5poiibat3s6mwgknqie\",\"Closing Prayer\",\"//lh6.googleusercontent.com/ZJev61hS70xCwTr1QNVTd4sX7TA4i6ZbvXdmcO3eMD7nlJfCG1aVSBDixXrrkwVBx5qcDzMur9c\",\"Goa Gil\",\"Goa Gil / Shri Maharaj\",\"Various Artists\",\"closing prayer\",\"goa gil\",\"goa gil / shri maharaj\",\"various artists\",,,,11000,11,,1,,,,,,0,-1,0,0,,\"Tc7neuug5poiibat3s6mwgknqie\",\"Tc7neuug5poiibat3s6mwgknqie\",5,,,\"Bxbfmoe5elg5fkwrhmfiz3lfxcq\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tpz4zffg6hcv2tv4l26kldjr7rm\",\"Freaks of Nature - White Grizzley\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"freaks of nature - white grizzley\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,387000,7,,1,,,,,,0,-1,0,0,,\"Tpz4zffg6hcv2tv4l26kldjr7rm\",\"Tpz4zffg6hcv2tv4l26kldjr7rm\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tvxg42tbddvqxnhvkcd3jy7h6hu\",\"Aum\",\"//lh6.googleusercontent.com/ZJev61hS70xCwTr1QNVTd4sX7TA4i6ZbvXdmcO3eMD7nlJfCG1aVSBDixXrrkwVBx5qcDzMur9c\",\"Goa Gil\",\"Goa Gil / Shri Maharaj\",\"Various Artists\",\"aum\",\"goa gil\",\"goa gil / shri maharaj\",\"various artists\",,,,22000,1,,1,,,,,,0,-1,0,0,,\"Tvxg42tbddvqxnhvkcd3jy7h6hu\",\"Tvxg42tbddvqxnhvkcd3jy7h6hu\",5,,,\"Bxbfmoe5elg5fkwrhmfiz3lfxcq\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tio7n5g5n7sx4mon3efjjlnnjm4\",\"Polyphonia - Challo\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"polyphonia - challo\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,407000,4,,1,,,,,,0,-1,0,0,,\"Tio7n5g5n7sx4mon3efjjlnnjm4\",\"Tio7n5g5n7sx4mon3efjjlnnjm4\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"T7dshy2gmxhft4v6ccoha4jlakm\",\"Om Mahakali\",\"//lh4.googleusercontent.com/d04AdZ9gDGIobgMgToyscfagwhZo51i4IhobKImqSP0-m9pQ_TWDI92EOJzF8EYRK9_PEx7DDg\",\"Goa Gil\",\"Goa Gil / Kali Yuga\",\"Various Artists\",\"om mahakali\",\"goa gil\",\"goa gil / kali yuga\",\"various artists\",,,,16000,1,,1,,,,,,0,-1,0,0,,\"T7dshy2gmxhft4v6ccoha4jlakm\",\"T7dshy2gmxhft4v6ccoha4jlakm\",5,,,\"Brng3jgw6phamiqlnvaqujwcaqm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
]\
]\
]\
]\
]\
";

static const char testAlbumFetch[] = "\
[[0]\
,[[[,\"Worldbridger\",\"Goa Gil\",\"http://lh3.ggpht.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",0,0,[[\"Tbnvxo3tijzmvfuefzr4xvedwk4\",\"Horror Place -The Search For Truth\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"horror place -the search for truth\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,453000,1,,1,,,,,,0,-1,0,0,,\"Tbnvxo3tijzmvfuefzr4xvedwk4\",\"Tbnvxo3tijzmvfuefzr4xvedwk4\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tajin3kv7ae5wtbirggpkotgkem\",\"Orestis - Contact With Peace\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"orestis - contact with peace\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,377000,2,,1,,,,,,0,-1,0,0,,\"Tajin3kv7ae5wtbirggpkotgkem\",\"Tajin3kv7ae5wtbirggpkotgkem\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tga3aeodvc6b55her7gbxil5eie\",\"The Nommos - Between Worlds\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"the nommos - between worlds\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,413000,3,,1,,,,,,0,-1,0,0,,\"Tga3aeodvc6b55her7gbxil5eie\",\"Tga3aeodvc6b55her7gbxil5eie\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tio7n5g5n7sx4mon3efjjlnnjm4\",\"Polyphonia - Challo\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"polyphonia - challo\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,407000,4,,1,,,,,,0,-1,0,0,,\"Tio7n5g5n7sx4mon3efjjlnnjm4\",\"Tio7n5g5n7sx4mon3efjjlnnjm4\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tpzocrgtohekegwixnxozcujwh4\",\"Kin Dza Dza - 9 Energy Transformers\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"kin dza dza - 9 energy transformers\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,353000,5,,1,,,,,,0,-1,0,0,,\"Tpzocrgtohekegwixnxozcujwh4\",\"Tpzocrgtohekegwixnxozcujwh4\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Twvxj5tr6mtrwflsx2rdcwfzwfu\",\"Stranger - The Worldbridger\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"stranger - the worldbridger\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,312000,6,,1,,,,,,0,-1,0,0,,\"Twvxj5tr6mtrwflsx2rdcwfzwfu\",\"Twvxj5tr6mtrwflsx2rdcwfzwfu\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tpz4zffg6hcv2tv4l26kldjr7rm\",\"Freaks of Nature - White Grizzley\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"freaks of nature - white grizzley\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,387000,7,,1,,,,,,0,-1,0,0,,\"Tpz4zffg6hcv2tv4l26kldjr7rm\",\"Tpz4zffg6hcv2tv4l26kldjr7rm\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tah26tu5abvt34ax3rborkxmcnm\",\"Savage Scream - Shadow Walker\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"savage scream - shadow walker\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,418000,8,,1,,,,,,0,-1,0,0,,\"Tah26tu5abvt34ax3rborkxmcnm\",\"Tah26tu5abvt34ax3rborkxmcnm\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tut2an427pk2tnuarp5hxriwhwy\",\"Kulu - Virtual Revolution\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"kulu - virtual revolution\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,369000,9,,1,,,,,,0,-1,0,0,,\"Tut2an427pk2tnuarp5hxriwhwy\",\"Tut2an427pk2tnuarp5hxriwhwy\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"T2opkam5ui3dhr5d4nmgjngnsw4\",\"Cosmo vs Cerebral Factory - Reality Hackers\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"cosmo vs cerebral factory - reality hackers\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,400000,10,,1,,,,,,0,-1,0,0,,\"T2opkam5ui3dhr5d4nmgjngnsw4\",\"T2opkam5ui3dhr5d4nmgjngnsw4\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
,[\"Tgzczfq7tupo4dogvde7k5bbf2a\",\"Sonik Scizzor - Nature's Roar\",\"//lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy\",\"Goa Gil\",\"Worldbridger\",\"Goa Gil\",\"sonik scizzor - nature's roar\",\"goa gil\",\"worldbridger\",\"goa gil\",,,,440000,11,,1,,,,,,0,-1,0,0,,\"Tgzczfq7tupo4dogvde7k5bbf2a\",\"Tgzczfq7tupo4dogvde7k5bbf2a\",5,,,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",\"Abrieyefqd7zyfsnd7ttway5qgy\",,,,,2,[]\
]\
]\
,\"B6o7ehs7mx4akcgi3uiw4s3nehm\",[]\
,2007,\"Abrieyefqd7zyfsnd7ttway5qgy\",[[,\"Atngmlxlixottlthpcypidmxlu4\",\"Deadmau5\",0,[]\
,\"http://lh3.ggpht.com/REvYUg3fVpmAGQ-vSI7jPpoAbPuCREQcsHbranRdPi8QPbl6I9qwHAOhJd2CrgT4X1ysSDCOhA\",,,[]\
,[]\
,,[]\
]\
,[,\"Antiemo7xwnzofue2ge7hybvpt4\",\"Bassnectar\",0,[]\
,\"http://lh5.ggpht.com/hOl8iK19V3GsTY1Oplg3i8pqk36AuxJTUxpp-rxWw75nMkX_jhnLj-h--Mlx1uHausUI2Q1rXw\",,,[]\
,[]\
,,[]\
]\
,[,\"Adhiec2ed2lvaglrmt52f3xu3ie\",\"The Chemical Brothers\",0,[]\
,\"https://lh5.ggpht.com/uZcFeibx10B3WU4Ttz1ft8W4GKnY1aXoBds4GrklnvzVM8OTHYCpufc0eic9W1s5oYGWgaW6aw\",,,[]\
,[]\
,,[]\
]\
,[,\"Afp2gizke2x5gjmweeutvncznq4\",\"Infected Mushroom\",0,[]\
,\"http://lh6.ggpht.com/kaiBMYZ88bBDGUowdVjgFCNAx_v80-YzUKYWyd2nYylKPTu5IwerXJcMR4LAQgJYMS8wZ0jtsA\",,,[]\
,[]\
,,[]\
]\
,[,\"Aruxfsh75imhlfzbzferobgbghm\",\"Crystal Method\",0,[]\
,\"https://lh6.ggpht.com/x_vdrM0S9uQYyIwLZlVeTOdU-78wP8iNJQhdNXKPBrT7rH6wQB_PplM4wUBT5zQh_nEfDtNThQ\",,,[]\
,[]\
,,[]\
]\
,[,\"A2fq3wwz25xla7czwcrizooqyry\",\"Noisia\",0,[]\
,\"http://lh4.ggpht.com/sbKj4sHZvhL9orKHFNSO2sGun5TrKvRaHlVojPM7sYbn6-PDkXGD3vT87pGpUEF_eyr5X4mZ\",,,[]\
,[]\
,,[]\
]\
,[,\"Agzf4r7d54azste62qh6z2q7bcy\",\"Tiësto\",0,[]\
,\"http://lh5.ggpht.com/R1qwTrlseTXP666UKjtfBJ7I6pMWd_ltheb8WZ7GpsKNEB5kPkBKR83-Jtg7Pcmah2-t-G4v7w\",,,[]\
,[]\
,,[]\
]\
,[,\"At43tgdn2zrm3bhbp2uky5626ni\",\"Flying Lotus\",0,[]\
,\"http://lh6.ggpht.com/AHcNPIYDfw8rIMC-ef3QWW-AQ0Xsd2qfgTuIwMyqDMLAWgJ353YYvIW8LLELBgmLq8Y7kippVQ8\",,,[]\
,[]\
,,[]\
]\
,[,\"Ap6exarirw2ls3jjrhnf5p2lcvm\",\"Daft Punk\",0,[]\
,\"http://lh6.ggpht.com/o2uJ47AczwwIQFOMycSXpBbKSjbAvV_1wv3NHJweMnb0fV5iolbP7lGiUt_AguS_iXy07NRK0TI\",,,[]\
,[]\
,,[]\
]\
,[,\"Adbugxnsjn4gptoljajngyu5h3q\",\"Skream\",0,[]\
,\"http://lh5.ggpht.com/TQTr99ZsCcuxeMu04ACjHqtViydy05px8i2imb8M__g87XQ0W-Yj8df1xTpHH9kyAqzW6Z4o\",,,[]\
,[]\
,,[]\
]\
,[,\"Aq5xe3lznblgooedgsfkkkgn6e4\",\"Mr. Oizo\",0,[]\
,\"http://lh4.ggpht.com/fg0O29SH7gMx_Be_FDuI7DvukfxsZdQcfYG7-X1koMrf09Y1NT2lBdM70UxLxVJriP0EJJSV\",,,[]\
,[]\
,,[]\
]\
,[,\"Ao62kujeondku3c4pa5lumsnqd4\",\"DJ Icey\",0,[]\
,\"http://lh4.ggpht.com/kh0eA_fb6nTm67y-hUE9zK05XKS53YldPzwgEHRIFQ4C--exRRYmrQDFzZU-53Lc7CfQpWKy\",,,[]\
,[]\
,,[]\
]\
,[,\"A7ihjt5dn2627dcioikmbhlhjka\",\"Modeselektor, PVT\",0,[]\
,\"http://lh5.ggpht.com/rhFxpPcYgYGKF60RieL3jXdjc_9KG4bijv-IdPV5s7h3ohX03isqyL0s0SUMgmazvrzfFn8yl8s\",,,[]\
,[]\
,,[]\
]\
,[,\"Az6ci2oussoklhglvbszzvpvy2e\",\"Boys Noize\",0,[]\
,\"http://lh3.ggpht.com/4U1buz-Cm7akRzDB7NqTdYxIB-0hLpuHacwq7LrsFB38UpkHQGg3vHVcu5oYg7O0SBZPB5Ey\",,,[]\
,[]\
,,[]\
]\
,[,\"Ayoidtvwv3jchewjeahrw3vuw7i\",\"The Prodigy\",0,[]\
,\"https://lh5.ggpht.com/csIir7Hp3uZHT_cnWiiizOBCmVyzkJJXIOjPIRlhrwXeAXsvFNLQP6MjCKqJn4GtwDn8EU4oS6o\",,,[]\
,[]\
,,[]\
]\
,[,\"Arwpidedwzzivb36hmtcd7fsgc4\",\"Timo Maas, Santos\",0,[]\
,\"http://lh6.ggpht.com/Jjg6wVpwePP39QpXlCEcZSP6mnwqVcV40S91oUsSw-JEI6OV-VDZcIiKEeBixM0JCUnx2Qeo\",,,[]\
,[]\
,,[]\
]\
,[,\"Axs7ndoga46xcsggyj3mgtdtuiq\",\"Benga\",0,[]\
,\"http://lh4.ggpht.com/wDrLcHILthmroKFJyD12ZlF9VhXZuehUrnl5q8Krg1gAe-L-a8-PpR5TyP5CORybGmmGOEfIhA\",,,[]\
,[]\
,,[]\
]\
,[,\"Aha73h4qadkud7tda2bux5lvrk4\",\"Tiga\",0,[]\
,\"http://lh3.ggpht.com/GqC9MSH9DFqMEegIQ_nQ4imLQ5LKIERTbAmlnzh4u4CZPP1h2hIye_hi_iqJnTD8-t9J296OHw\",,,[]\
,[]\
,,[]\
]\
,[,\"Amnp4kbyhk55v3iz7c2cz262vm4\",\"Black Sun Empire\",0,[]\
,\"http://lh3.ggpht.com/S9y3lW1HRX6scZkv09Oem7CiM-h_mUVLxNd1Xphm-3O6KKHVwgTDIoaYkIRw5EkysgXz6PDc3g\",,,[]\
,[]\
,,[]\
]\
,[,\"Al2gwn5dkfmqs2w7d7lqbxdahm4\",\"Plump DJs\",0,[]\
,\"http://lh5.ggpht.com/kJNKuI_8DE8j94y3bZQhUr4EDoTipgmotVPnPyYk75z-feN0Gi79V9PvASTh0YHTAIrSJFtdgg\",,,[]\
,[]\
,,[]\
]\
]\
,,2]\
]\
]\
]\
";

static const char testRadioList[] = "\
[[0]\
,[[[\"856b4f54-098e-3976-b07a-29b40bc79d63\",\"Tales of the Sands\",\"\",[\"http://lh6.ggpht.com/gXndFPhP8a_AheOXFD8ggh6F-Dz3MpQy_6vnjxFPBvy3nc2171xB6RE977XO0P_LwLi5F4YiadI\"]\
,[]\
,1370273931167000,[1,\"Tetdsk3li2vyuaxinlgt2yf5knq\"]\
]\
,[\"9f6d57e1-a40f-3a1d-a7dd-54c3925e5c5a\",\"Voyage to India (Buddha Sunset Del Mar Vocal Mix)\",\"\",[\"http://lh6.ggpht.com/YBQB_LVTj-hcn341sbtftPeMCzw1eWUupjut1lhtQCCA9xDtSJS9cr8LnPtGeh0FSiQ3VcYWHO8\"]\
,[]\
,1370196799579000,[2,\"Bi3jc6vvvl444odwg3xmbe4epxu\"]\
]\
,[\"789b6478-4a2d-3904-a368-cde8bcb6ed10\",\"Front Line Assembly\",\"\",[\"http://lh3.ggpht.com/UnIjkhPCAf8niStaRanv-JEjeUTdJqQUfh1JJ7yybAj3acYDaWExApmD8F_b1NQGf8Mz6CcQmQ\"]\
,[]\
,1370101147522000,[3,\"Aizgtplesb5jtfqx3bdzqcyzqgq\"]\
]\
,[\"3ebade0f-34f4-3fe4-b5c5-4e4a1f6231b2\",\"Sunrider\",\"\",[\"http://lh5.ggpht.com/aHyHHDv9gyNc3xYgT24uo9swDthdDWaphotLcsUbUSFxTaQj7oSv9ld5K1_CuMpBPfdkPCZcFQ\"]\
,[]\
,1370101080422000,[3,\"A3bytfqj3lj3d44yp2x7monjm4q\"]\
]\
,[\"6149ef04-5e06-3a4e-9a3f-26dd96264e0e\",\"Fatali\",\"\",[\"http://lh3.ggpht.com/UPrFSMJb4Scn-MI9C-inD1cMRbmNwqppINrJZiFXxEr8lYFta4Fp68DYmiKsjFZuRDhVPN34\"]\
,[]\
,1370101047979000,[3,\"Aruy3qi3tsn72yur674naashjuq\"]\
]\
,[\"05736c76-1949-31af-a55b-a32cb3d8bb2a\",\"Weapon\",\"\",[\"http://lh6.ggpht.com/PUeZADpkunrXYh-NYWFAsANxrqT2hFobgjJXp6c5NPkfQpobrkEqqpUy5fj8dd6TF1mRaecd\"]\
,[]\
,1370100968947000,[2,\"Bvk5up2by572bg5du4kqb6qvq6a\"]\
]\
,[\"317b80e8-d948-3452-9532-00c4cccb62a7\",\"Amphetamine Logic\",\"\",[\"http://lh4.ggpht.com/eQtdA8N2Z0JBWOFtTNujBPqyXfckeD9TImW-W_RXXqxNt1g61RPck9S7QMBwyEge83uYiaIt3Kw\"]\
,[]\
,1370100880699000,[0,\"32f384f2-923b-32c4-a291-df8666f77e4f\"]\
]\
]\
,[[\"\",\"\",\"\",[]\
,[[\"0e65a990-f72f-3892-8ade-0349be2515df\",\"Father Don't Cry\",\"//lh4.googleusercontent.com/zKZVvrDIVR6pzwZIKZaVwTit5r-hN6jQLVkhBktxANddJ4z3PP4BF_FVZhzbzQjaP8HvazKwMA\",\"Doubting Thomas\",\"The Infidel\",\"Doubting Thomas\",\"father don't cry\",\"doubting thomas\",\"the infidel\",\"doubting thomas\",\"\",\"\",\"\",315634,3,13,1,0,2007,0,,,9,0,1369232380706891,1370079742949752,,,\"Tc7z3u7pa3sfr4go4r4g5dxtcli\",6,\"\",,\"B6cvvgw5fp5fc46s5mzfyookrkm\",\"A4sbvaglmhru3yhew5h2tfjirli\",320,1369232380705000,,,,[]\
]\
]\
,0,[0,\"0e65a990-f72f-3892-8ade-0349be2515df\",\"Tc7z3u7pa3sfr4go4r4g5dxtcli\"]\
]\
,[\"\",\"\",\"\",[]\
,[[\"32f384f2-923b-32c4-a291-df8666f77e4f\",\"Amphetamine Logic\",\"//lh4.googleusercontent.com/eQtdA8N2Z0JBWOFtTNujBPqyXfckeD9TImW-W_RXXqxNt1g61RPck9S7QMBwyEge83uYiaIt3Kw\",\"Sisters Of Mercy\",\"First And Last And Always (Remastered \\u0026 Expanded)\",\"Sisters Of Mercy\",\"amphetamine logic\",\"sisters of mercy\",\"first and last and always (remastered \\u0026 expanded)\",\"sisters of mercy\",,,,294000,9,,1,,,0,,,70,-1,1368814119424843,1370093791475362,,\"Tepwwn35wbefhanl355ofqrc2sm\",\"Tepwwn35wbefhanl355ofqrc2sm\",7,,,\"Bj2ciaqzvqzycvw3otzxmo4qdsa\",\"A5oh3vbdoiew3743wqupz7tb26y\",,1368814119424000,,,2,[]\
]\
]\
,0,[0,\"32f384f2-923b-32c4-a291-df8666f77e4f\",\"Tepwwn35wbefhanl355ofqrc2sm\"]\
]\
,[\"\",\"\",\"\",[]\
,[[\"2f265354-f030-3b54-b134-b29581c43d4c\",\"The Vulture\",\"//lh4.googleusercontent.com/id2VpnXLqSHlgvGl79GVLAXYIZShHW9RqeSry7j2VgPjZWE7Iujm8QRkVQyL0kqDQdi9dmp2\",\"In Slaughter Natives\",\"Flowers Made Of Snow\",\"Various Artists\",\"the vulture\",\"in slaughter natives\",\"flowers made of snow\",\"various artists\",,,,295000,3,,1,,,0,,,2,5,1370079849787882,1370079863877723,,\"Ttwh42v23tat5hv6vrvwwfbhxau\",\"Ttwh42v23tat5hv6vrvwwfbhxau\",7,,,\"Bautzd3yplfj4k4haaic7s5j6wu\",\"At6iz5d3vujmtfchkqp2g3sdyoq\",,1370079849787000,,,1,[]\
]\
]\
,0,[0,\"2f265354-f030-3b54-b134-b29581c43d4c\",\"Ttwh42v23tat5hv6vrvwwfbhxau\"]\
]\
,[\"\",\"\",\"\",[]\
,[[\"50ead1ff-7a1f-3017-98cc-6a421c8428a7\",\"Baltabolt\",\"//lh6.googleusercontent.com/-0q89unsldGlbE-ni4kU-tK3eHwQQexjmPOFnuX_e8fBAM2Dcqc-8SfZUBug2m_kxrB2f2WW\",\"Korog\",\"Próbáld újra\",\"Korog\",\"baltabolt\",\"korog\",\"próbáld újra\",\"korog\",,,,142000,6,,1,,2001,0,,,19,-1,1368861241489351,1369932397503028,,\"Tbydoaqa6ymxayl2p2gu4bhyra4\",\"Tbydoaqa6ymxayl2p2gu4bhyra4\",7,,,\"Bcakiedai3okoy2qfuhsbz5nuly\",\"Astvb3pccijqj2rwefo2qitxtyy\",,1368861241488000,,,2,[]\
]\
]\
,0,[0,\"50ead1ff-7a1f-3017-98cc-6a421c8428a7\",\"Tbydoaqa6ymxayl2p2gu4bhyra4\"]\
]\
,[\"\",\"\",\"\",[]\
,[[\"986a56c0-a25f-3757-a330-6e500c82ea3d\",\"Carbon-Based Anatomy\",\"//lh4.googleusercontent.com/JfGPH22SmVZRKxA7PWsDjSTAx9iGo5o2JX73RokesvzzEFogeQmZwxaom5p2h_7UEQ1LlZR0Uw\",\"Cynic\",\"Carbon-Based Anatomy\",\"Cynic\",\"carbon-based anatomy\",\"cynic\",\"carbon-based anatomy\",\"cynic\",\"\",\"Metal\",\"metal\",384336,2,6,1,1,2011,0,,,2,5,1369232354474039,1369944321046189,,\"Tqisganmcseaburnucxot4lqju4\",\"Tqisganmcseaburnucxot4lqju4\",6,\"\",,\"Bvhzidxrkyzvaynj7jd5dzzjje4\",\"Aml25lplroixh5cibftqhyudjqa\",320,1369232354473000,,,,[]\
]\
]\
,0,[0,\"986a56c0-a25f-3757-a330-6e500c82ea3d\",\"Tqisganmcseaburnucxot4lqju4\"]\
]\
]\
]\
]\
";

static const char testRadioFetch[] = "\
[[0]\
,[[[\"9d28df8f-9bb4-307c-9488-1ef6eabf5db3\",\"Voyage to India (Buddha Sunset Del Mar Vocal Mix)\",\"//lh6.googleusercontent.com/YBQB_LVTj-hcn341sbtftPeMCzw1eWUupjut1lhtQCCA9xDtSJS9cr8LnPtGeh0FSiQ3VcYWHO8\",\"Ragi\",\"Voyage to India (Buddha Sunset Del Mar Vocal Mix)\",\"Ragi\",\"voyage to india (buddha sunset del mar vocal mix)\",\"ragi\",\"voyage to india (buddha sunset del mar vocal mix)\",\"ragi\",,,,355000,1,,1,,2013,0,,,10,-1,1369933550535277,1370093791879745,,\"T4y3lsyneokaud4svmidmovd2um\",\"T4y3lsyneokaud4svmidmovd2um\",7,,,\"Bi3jc6vvvl444odwg3xmbe4epxu\",\"Abiizpxku7myaz72lufqvneoynu\",,1369933550534000,,,2,[]\
]\
,[\"20fda0e1-b751-3646-9c76-7a4e641f7a63\",\"Nagual Tone\",\"//lh4.googleusercontent.com/zKZVvrDIVR6pzwZIKZaVwTit5r-hN6jQLVkhBktxANddJ4z3PP4BF_FVZhzbzQjaP8HvazKwMA\",\"Doubting Thomas\",\"The Infidel\",\"Doubting Thomas\",\"nagual tone\",\"doubting thomas\",\"the infidel\",\"doubting thomas\",\"\",\"\",\"\",241891,7,13,1,0,2007,0,,,4,0,1369232385021479,1370079624641151,,,\"Thtsih7vob5dlg7m5obejsmdxua\",6,\"\",,\"B6cvvgw5fp5fc46s5mzfyookrkm\",\"A4sbvaglmhru3yhew5h2tfjirli\",320,1369232385020000,,,,[]\
]\
,[\"91c4a45a-833e-3c49-9da0-e977f888701c\",\"Try\",\"//lh6.googleusercontent.com/Vb3R85yubfbqL-zxDcjIeMcvJVg_eNh3zg9AviZfPrg5F1YigJEbTiM-IH8Yot341btu9dhO56M\",\"P!nk\",\"The Truth About Love\",\"P!nk\",\"try\",\"p!nk\",\"the truth about love\",\"p!nk\",,,,247000,3,,1,,2012,0,,,4,-1,1369930711709994,1369930973171635,,\"Tkx4chmpcuioztj2x6xrhq7wn2q\",\"Tkx4chmpcuioztj2x6xrhq7wn2q\",7,,,\"Bldquheeunrtwjsrnnntc4bgfiq\",\"Asva43fw276tfwhxhmyoa3ggv5y\",,1369930711709000,,,2,[]\
]\
,[\"047bd814-c62b-3f97-a344-68f8da0aeede\",\"Welcome To Paradise\",\"//lh6.googleusercontent.com/6ipaO-TkrU6bZK6VFf-1Z0mIktoF5KaF6b_3uNPu8XqtAOnGKClph9QP8L2tm4p3_Q4E9Tn_xg\",\"Front 242\",\"FRONT BY FRONT\",\"Front 242\",\"welcome to paradise\",\"front 242\",\"front by front\",\"front 242\",,,,317000,10,,1,,1992,0,,,1,-1,1369931540226111,1369931540226111,,\"Trqzaxpzmnuktxu4ufeplvisq2a\",\"Trqzaxpzmnuktxu4ufeplvisq2a\",7,,,\"B3gm7itverxkkuq27dr7nclfn6y\",\"Apx5xxac7lpykujus3a6fjugyia\",,1369931540225000,,,2,[]\
]\
,[\"3fba960c-4452-38f1-bcff-65a930f76ed6\",\"Hieroglyph\",\"//lh4.googleusercontent.com/JfGPH22SmVZRKxA7PWsDjSTAx9iGo5o2JX73RokesvzzEFogeQmZwxaom5p2h_7UEQ1LlZR0Uw\",\"Cynic\",\"Carbon-Based Anatomy\",\"Cynic\",\"hieroglyph\",\"cynic\",\"carbon-based anatomy\",\"cynic\",\"\",\"Metal\",\"metal\",147877,6,6,1,1,2011,0,,,0,0,1369232356626938,1369944630043543,,\"Tohypr6ttew2rbgiealkh66emum\",\"Tohypr6ttew2rbgiealkh66emum\",6,\"\",,\"Bvhzidxrkyzvaynj7jd5dzzjje4\",\"Aml25lplroixh5cibftqhyudjqa\",320,1369232356626000,,,,[]\
]\
,[\"5538c165-1008-378f-a3a5-6694b3e98a7d\",\"Saved\",\"//lh4.googleusercontent.com/zKZVvrDIVR6pzwZIKZaVwTit5r-hN6jQLVkhBktxANddJ4z3PP4BF_FVZhzbzQjaP8HvazKwMA\",\"Doubting Thomas\",\"The Infidel\",\"Doubting Thomas\",\"saved\",\"doubting thomas\",\"the infidel\",\"doubting thomas\",\"\",\"\",\"\",234002,9,13,1,0,2007,0,,,11,5,1369232393371831,1369935842049216,,,\"Tur33dkcrrl73bo7nuguclavlwa\",6,\"\",,\"B6cvvgw5fp5fc46s5mzfyookrkm\",\"A4sbvaglmhru3yhew5h2tfjirli\",320,1369232393370000,,,,[]\
]\
,[\"33ae8330-0890-3582-bdc8-791403b83bc9\",\"Intro\",\"//lh5.googleusercontent.com/vh6EXw0dveZBb3swr9K9XmrFKj5hKV2yxAvUZavRiP9QQDSzrdMXmG87JZD8VYOyHEY2GkF466s\",\"Ice Ages\",\"Buried Silence\",\"Ice Ages\",\"intro\",\"ice ages\",\"buried silence\",\"ice ages\",\"\",\"Dark Ambient\",\"dark ambient\",203309,1,0,0,0,2008,0,,,0,0,1369232405698696,1369232405698696,,\"Te7nxxwclhlzuu7aqzac63siovq\",\"Te7nxxwclhlzuu7aqzac63siovq\",6,\"\",,\"B3tlpbjfynv6ogrwnfmlge3t3ny\",\"Awvnvpscsnk6vag5s3vjhoplnuq\",320,1369232405698000,,,,[]\
]\
,[\"9d427a5d-47d1-365c-8ebd-f08c974f8937\",\"Headhunter V1.0\",\"//lh4.googleusercontent.com/yec7mrI6pYHaHwhBSDeJK-sd_5rRjDbafxLG8a9wXFeYHWQiuvFyTJDBP57kBsWD7rHBCtv7uSw\",\"Front 242\",\"FRONT BY FRONT\",\"Front 242\",\"headhunter v1.0\",\"front 242\",\"front by front\",\"front 242\",,,,301000,11,,1,,1992,0,,,0,-1,1369931540226111,1369931540226111,,\"Tchvxta3cdhstdkevv6jru7dimi\",\"Tchvxta3cdhstdkevv6jru7dimi\",7,,,\"B3gm7itverxkkuq27dr7nclfn6y\",\"Apx5xxac7lpykujus3a6fjugyia\",,1369931540225000,,,2,[]\
]\
,[\"4db48dce-f9be-3c6f-943f-e40562bac8a8\",\"Regret\",\"//lh5.googleusercontent.com/vh6EXw0dveZBb3swr9K9XmrFKj5hKV2yxAvUZavRiP9QQDSzrdMXmG87JZD8VYOyHEY2GkF466s\",\"Ice Ages\",\"Buried Silence\",\"Ice Ages\",\"regret\",\"ice ages\",\"buried silence\",\"ice ages\",\"\",\"Dark Ambient\",\"dark ambient\",349332,3,0,0,0,2008,0,,,3,0,1369232408512458,1369932048376544,,,\"Tfsm7axujardb3l63yeno3rlaju\",6,\"\",,\"B3tlpbjfynv6ogrwnfmlge3t3ny\",\"Awvnvpscsnk6vag5s3vjhoplnuq\",320,1369232408511000,,,,[]\
]\
,[\"32f384f2-923b-32c4-a291-df8666f77e4f\",\"Amphetamine Logic\",\"//lh4.googleusercontent.com/eQtdA8N2Z0JBWOFtTNujBPqyXfckeD9TImW-W_RXXqxNt1g61RPck9S7QMBwyEge83uYiaIt3Kw\",\"Sisters Of Mercy\",\"First And Last And Always (Remastered \\u0026 Expanded)\",\"Sisters Of Mercy\",\"amphetamine logic\",\"sisters of mercy\",\"first and last and always (remastered \\u0026 expanded)\",\"sisters of mercy\",,,,294000,9,,1,,,0,,,70,-1,1368814119424843,1370093791475362,,\"Tepwwn35wbefhanl355ofqrc2sm\",\"Tepwwn35wbefhanl355ofqrc2sm\",7,,,\"Bj2ciaqzvqzycvw3otzxmo4qdsa\",\"A5oh3vbdoiew3743wqupz7tb26y\",,1368814119424000,,,2,[]\
]\
,[\"d65ea3d9-be47-38d3-b0e2-e91ad8c2e26e\",\"Vanished\",\"//lh3.googleusercontent.com/-YcNWQk9K9B5xgMlL2IOhAEEehvJBf_BBXGZGpYX2oYTigEGw6SRTmB1UfL_ycNnJ0LDc91Q8A\",\"Front Line Assembly\",\"Civilization\",\"Front Line Assembly\",\"vanished\",\"front line assembly\",\"civilization\",\"front line assembly\",,,,385000,4,,1,,2004,0,,,0,-1,1370079973655291,1370079973655291,,\"Tc6bw67ycc2ocxxtsltnhygigga\",\"Tc6bw67ycc2ocxxtsltnhygigga\",7,,,\"Bjxzwsgkooiu7rvd4pdriifqcmm\",\"Aizgtplesb5jtfqx3bdzqcyzqgq\",,1370079973654000,,,2,[]\
]\
,[\"50ead1ff-7a1f-3017-98cc-6a421c8428a7\",\"Baltabolt\",\"//lh6.googleusercontent.com/-0q89unsldGlbE-ni4kU-tK3eHwQQexjmPOFnuX_e8fBAM2Dcqc-8SfZUBug2m_kxrB2f2WW\",\"Korog\",\"Próbáld újra\",\"Korog\",\"baltabolt\",\"korog\",\"próbáld újra\",\"korog\",,,,142000,6,,1,,2001,0,,,19,-1,1368861241489351,1369932397503028,,\"Tbydoaqa6ymxayl2p2gu4bhyra4\",\"Tbydoaqa6ymxayl2p2gu4bhyra4\",7,,,\"Bcakiedai3okoy2qfuhsbz5nuly\",\"Astvb3pccijqj2rwefo2qitxtyy\",,1368861241488000,,,2,[]\
]\
,[\"b8f9e961-7c5b-3be8-829c-6a6874708956\",\"Headhunter\",\"//lh4.googleusercontent.com/fOERJlADTZoejucMqWTJNf2qk4pyy4oJkmlqdxwcpc6hSgy-hMYEoXZwyYuuqBDYy28Qj_4V8WU\",\"Front 242\",\"Re: Connected 2.0\",\"Various Artists\",\"headhunter\",\"front 242\",\"re: connected 2.0\",\"various artists\",,,,267000,10,,1,,,0,,,1,-1,1369931488962265,1369931488962265,,\"Ttx7y6gic7vdxwqawdtkpfqatay\",\"Ttx7y6gic7vdxwqawdtkpfqatay\",7,,,\"Br5oiatrilsr5q62j4amdv2gt2m\",\"Apx5xxac7lpykujus3a6fjugyia\",,1369931488961000,,,2,[]\
]\
,[\"c2bc4691-9ef0-320e-aa3a-b4bcbd2d249f\",\"Amidst The Coals\",\"//lh4.googleusercontent.com/JfGPH22SmVZRKxA7PWsDjSTAx9iGo5o2JX73RokesvzzEFogeQmZwxaom5p2h_7UEQ1LlZR0Uw\",\"Cynic\",\"Carbon-Based Anatomy\",\"Cynic\",\"amidst the coals\",\"cynic\",\"carbon-based anatomy\",\"cynic\",\"\",\"Metal\",\"metal\",131420,1,6,1,1,2011,0,,,0,0,1369232367388248,1369232367388248,,\"Tirjk23f4hlp23qh6rcw2ol5fka\",\"Tirjk23f4hlp23qh6rcw2ol5fka\",6,\"\",,\"Bvhzidxrkyzvaynj7jd5dzzjje4\",\"Aml25lplroixh5cibftqhyudjqa\",320,1369232367387000,,,,[]\
]\
,[\"2f265354-f030-3b54-b134-b29581c43d4c\",\"The Vulture\",\"//lh4.googleusercontent.com/id2VpnXLqSHlgvGl79GVLAXYIZShHW9RqeSry7j2VgPjZWE7Iujm8QRkVQyL0kqDQdi9dmp2\",\"In Slaughter Natives\",\"Flowers Made Of Snow\",\"Various Artists\",\"the vulture\",\"in slaughter natives\",\"flowers made of snow\",\"various artists\",,,,295000,3,,1,,,0,,,2,5,1370079849787882,1370079863877723,,\"Ttwh42v23tat5hv6vrvwwfbhxau\",\"Ttwh42v23tat5hv6vrvwwfbhxau\",7,,,\"Bautzd3yplfj4k4haaic7s5j6wu\",\"At6iz5d3vujmtfchkqp2g3sdyoq\",,1370079849787000,,,1,[]\
]\
,[\"56abb454-e9a9-3a87-9df9-0eeb1631cc33\",\"Theme From Pressurehead\",\"//lh4.googleusercontent.com/zKZVvrDIVR6pzwZIKZaVwTit5r-hN6jQLVkhBktxANddJ4z3PP4BF_FVZhzbzQjaP8HvazKwMA\",\"Doubting Thomas\",\"The Infidel\",\"Doubting Thomas\",\"theme from pressurehead\",\"doubting thomas\",\"the infidel\",\"doubting thomas\",\"\",\"\",\"\",246071,12,13,1,0,2007,0,,,4,0,1369232378681273,1369935820202051,,,\"Tqtyjfrtp5ragwddmobz5qwgvkq\",6,\"\",,\"B6cvvgw5fp5fc46s5mzfyookrkm\",\"A4sbvaglmhru3yhew5h2tfjirli\",320,1369232378680000,,,,[]\
]\
,[\"ec052efb-77d7-3262-b88d-b960e5764982\",\"Não quero ver o fim\",\"//lh5.googleusercontent.com/xTZt9vgAUeE8v2gHVNhzeIBEv_mWXMGOoRG8jV0zcU6XrfUYcvAtd5PTKr-6dn__Gu-WSZTuKQ\",\"FF\",\"A música nasce\",\"FF\",\"não quero ver o fim\",\"ff\",\"a música nasce\",\"ff\",,,,190000,4,,1,,,0,,,1,-1,1369932851551930,1369932851551930,,\"T7k4gmmjgjr3oib2jpxkrfsp3ca\",\"T7k4gmmjgjr3oib2jpxkrfsp3ca\",7,,,\"Bkbnu2bjxo5bhjdbuxc4n4eoooa\",\"Aircuy2lyj2uxijhhqffn2kcu2e\",,1369932851550000,,,2,[]\
]\
,[\"e6549969-fd37-3f26-ae5e-711e4a2891a2\",\"One (A Team Rmx)\",,\"Astral Projection\",\"The Blissdom EP (11th Anniversary Limited Edition)\",\"\",\"one (a team rmx)\",\"astral projection\",\"the blissdom ep (11th anniversary limited edition)\",\"\",\"\",\"Psychedelic\",\"psychedelic\",533311,2,0,0,0,2010,0,,,3,0,1369232352242851,1370095401346126,,\"T2cbaosabzygcbccaqlwzt6oc3y\",\"T2cbaosabzygcbccaqlwzt6oc3y\",6,\"\",,\"Blpm6d5zy6hiq65c6rbjpq4xyn4\",\"Aqsemyiwjgye6qwfde37xm5czh4\",320,1369232352242000,,,,[]\
]\
,[\"84e20ee7-edfe-3cf9-83cb-020773802902\",\"Livin' On A Prayer\",\"//lh3.googleusercontent.com/uvfnXcztoUNrfCLOr14l6PpCHj0cP_rxXemPmDJyEzGFWpPV8bIcm_agQx3OFho33YW_pQfFrQ\",\"Bon Jovi\",\"Bon Jovi Greatest Hits\",\"Bon Jovi\",\"livin' on a prayer\",\"bon jovi\",\"bon jovi greatest hits\",\"bon jovi\",,,,250000,1,,1,,2010,0,,,3,-1,1369932889701602,1369933263179438,,\"Tfiy7ynt63tx6wvykpqui7vfege\",\"Tfiy7ynt63tx6wvykpqui7vfege\",7,,,\"B2g5yzqogtd3dakna5k3qptgjfy\",\"An4e5bnhixdhofvtjcwa2vxmwoe\",,1369932889701000,,,2,[]\
]\
,[\"9fc4c994-7dec-3dd7-9eba-ae1f0941a8b0\",\"The Great Escape\",\"//lh4.googleusercontent.com/JAuRxGGLdh5K7aGnnj9b-dbMct2kwArQuNPcxoz2FhWwPQF3NRzYHw_-f-OhYm1aSTOKsmS9bg\",\"P!nk\",\"The Truth About Love\",\"P!nk\",\"the great escape\",\"p!nk\",\"the truth about love\",\"p!nk\",,,,264000,13,,1,,2012,0,,,1,-1,1369931126261802,1369931180855161,,\"Tukct4yoegrs4pgyvqiqg7zzvri\",\"Tukct4yoegrs4pgyvqiqg7zzvri\",7,,,\"Bldquheeunrtwjsrnnntc4bgfiq\",\"Asva43fw276tfwhxhmyoa3ggv5y\",,1369931126261000,,,2,[]\
]\
,[\"c85f9cdc-2e0b-37f7-852b-71e6f44c5692\",\"Bija!\",\"//lh4.googleusercontent.com/JfGPH22SmVZRKxA7PWsDjSTAx9iGo5o2JX73RokesvzzEFogeQmZwxaom5p2h_7UEQ1LlZR0Uw\",\"Cynic\",\"Carbon-Based Anatomy\",\"Cynic\",\"bija!\",\"cynic\",\"carbon-based anatomy\",\"cynic\",\"\",\"Metal\",\"metal\",147407,3,6,1,1,2011,0,,,2,0,1369232351593892,1369931046593289,,,\"T6upjq44hsekxqhbb7lbogrow2q\",6,\"\",,\"Bvhzidxrkyzvaynj7jd5dzzjje4\",\"Aml25lplroixh5cibftqhyudjqa\",320,1369232351593000,,,,[]\
]\
,[\"61047bc4-dfea-383e-92c0-a6dd3143ea34\",\"Cure - A Forest - blank \\u0026 jones remix\",\"//lh3.googleusercontent.com/j-xDE4Es9D3Sv2x08ESvnVMP7PmXP05RIKrz3DcoLt-czhuPAhpZsdQtO53lTGyuUHnwRODXVg\",\"Blank and Jones feat Rob Smith\",\"A Forest (Original Mix)\",\"\",\"cure - a forest - blank \\u0026 jones remix\",\"blank and jones feat rob smith\",\"a forest (original mix)\",\"\",\"\",\"Other\",\"other\",411372,0,0,0,0,0,0,,,0,0,1369232361815858,1369232361815858,,,\"Trvrlrvo2j5wz4aaexxxc4bblzy\",6,\"\",,\"Bath2w7u2luanywd7hidpj5haxq\",\"Axwyv4jlkwxa77op7s7wyhfapeu\",320,1369232361815000,,,,[]\
]\
,[\"598f32be-01f0-33e2-9ee2-4fa061cc7e8b\",\"The History Of War (Feat. Fatali)(Fatali Rmx)\",,\"Astral Projection\",\"The Blissdom EP (11th Anniversary Limited Edition)\",\"\",\"the history of war (feat. fatali)(fatali rmx)\",\"astral projection\",\"the blissdom ep (11th anniversary limited edition)\",\"\",\"\",\"Psychedelic\",\"psychedelic\",511995,8,0,0,0,2010,0,,,0,0,1369232367325340,1369232367325340,,\"Tbv3hab57n5bbfzntiie7mal7wa\",\"Tbv3hab57n5bbfzntiie7mal7wa\",6,\"\",,\"Byta4mbkyfezn3bdpmhmnpgj5ri\",\"Aruy3qi3tsn72yur674naashjuq\",320,1369232367324000,,,,[]\
]\
,[\"94a3924c-7834-30bc-8594-fd54bb821b67\",\"Here Comes The Weekend (Explicit Version)\",\"//lh4.googleusercontent.com/7nh3587C7Py60qqV4PjzB4f5JMwfbmE2QDd0LLe9Y--U0dYELXqEnsQ-gwtSz8gaMRnwoXS4\",\"P!nk featuring Eminem\",\"The Truth About Love\",\"P!nk\",\"here comes the weekend (explicit version)\",\"p!nk featuring eminem\",\"the truth about love\",\"p!nk\",,,,264000,11,,1,,2012,0,,,1,-1,1369931115311792,1369931150007933,,\"Tkpvhyycz54n7c2h5col4zvomtq\",\"Tkpvhyycz54n7c2h5col4zvomtq\",7,,,\"Bldquheeunrtwjsrnnntc4bgfiq\",\"Asva43fw276tfwhxhmyoa3ggv5y\",,1369931115311000,,,1,[]\
]\
,[\"16b3124a-4132-388b-83c1-2acff29cc53d\",\"Testure (Live)\",\"//lh6.googleusercontent.com/rUbZ49ei383c9U7NAsKWyL_ueZG1GjFP7jHFEZk3qEAPOn9Ro1ZqTvGzTIsa0OcVrmdvBGX7TQ\",\"Skinny Puppy\",\"Doomsday (Back \\u0026 Forth Vol. 5 LIVE)\",\"Skinny Puppy\",\"testure (live)\",\"skinny puppy\",\"doomsday (back \\u0026 forth vol. 5 live)\",\"skinny puppy\",,,,307000,10,,1,,2001,0,,,1,-1,1370020600759537,1370067434798498,,\"Trc3qtfeul7ovaloxvjihyodaou\",\"Trc3qtfeul7ovaloxvjihyodaou\",7,,,\"B5c4v5uzmuktxo4nssscrkizoqy\",\"A7gkcirwl4lpnnfgmti3unznhgq\",,1370020600758000,,,2,[]\
]\
]\
,\"9f6d57e1-a40f-3a1d-a7dd-54c3925e5c5a\"]\
]\
";

static const char testCreatePlaylist[] = "\
[[0]\
,[\"34c9f85c-e0c8-47ba-9b67-2c1097f2b749\",\"AMaBXynY-ighmX4nKBdTpq6AcrFlOo-vlbYm563vBuLon-Z9Rs1rcaahLcRSiiPb_64RYnUdof9ccbmfiPCQgdhcWpxbHP-2Eg\\u003d\\u003d\",[[\"Tbv3hab57n5bbfzntiie7mal7wa\",,\"201ea04c-10a9-3794-ac16-4344227af3b7\"]\
,[\"T2t43bryun6ukufvifmsqkpsazu\",,\"bda37134-a82f-30c7-9a92-f48d312432b2\"]\
,[\"T2o7uvr5o44crtrp6oshewyajim\",,\"d27fa0ba-3136-36db-91a0-375b5111d475\"]\
,[\"Tg4tlrtu7ehglfq72serzbcrq7a\",,\"35888514-9632-3cec-8f54-09af675b988b\"]\
]\
]\
]";

static const char testAddToPlaylist[] = "\
{\"playlistId\":\"cc9d4548-0bec-4a41-a1f7-5bdd2257117c\",\"songIds\":[{\"playlistEntryId\":\"0e7e804a-1357-31d4-8cfd-f790d0915e28\",\"songId\":\"56abb454-e9a9-3a87-9df9-0eeb1631cc33\"},{\"playlistEntryId\":\"8ab36f88-dd86-3b1a-af47-666a8edc0bf1\",\"songId\":\"d65ea3d9-be47-38d3-b0e2-e91ad8c2e26e\"},{\"playlistEntryId\":\"7fd8020a-c176-319a-81a1-eab6ddc2d7aa\",\"songId\":\"9d28df8f-9bb4-307c-9488-1ef6eabf5db3\"},{\"playlistEntryId\":\"9fcd32c4-eb6d-3bdf-9cf8-f0a2514157b8\",\"songId\":\"2f265354-f030-3b54-b134-b29581c43d4c\"}]}\
";

static const char testAddToLibrary[] = "\
[[0]\
,[[]\
  ,[[\"4eae954c-d652-3e84-b7fd-0c2e6bad5936\",\"Ta5gne57p3n7fqv7tlwcit2mp6a\"]\
    ,[\"5b16fe5f-9242-35ad-a69e-fc238a884232\",\"T64eygqxved6qiozzc53esf5zzi\"]\
    ,[\"624aa5e9-264a-385a-ae1b-d50375e77997\",\"Thoaqvdlejxlhqtx7pk4bfyd64u\"]\
    ]\
  ]\
]";

namespace tut
{
    struct googleplayconceiverinfo
    {
    };
    
    typedef test_group<googleplayconceiverinfo> testgroup;
    typedef testgroup::object testobject;
    static testgroup shared_ptr_testgroup("GooglePlayConceiver");
    
    
    static void replaceOccurrences(string &str, const string &search, const string &replace)
    {
        size_t pos = 0;
        while ((pos = str.find(search,pos)) != std::string::npos) {
            str.replace(pos, search.length(), replace);
        }
    }

    static Json::Value parse(const string &aStr)
    {
        Json::Value value;
        Json::Reader reader;
        
        string str = aStr;
        // make it valid json
        replaceOccurrences(str, ",,", ",0,");
        replaceOccurrences(str, "[,", "[0,");
        replaceOccurrences(str, ",]", ",0]");
        replaceOccurrences(str, "[]", "0");
        
        bool success = reader.parse(str, value);
        ensure("jsonparse", success);
        return value;
    }
    
        
    template<>
    template<>
    void testobject::test<1>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testSearchReply), session);
        auto songs = conceiver.songs();

        ensure("songcount", songs.size() == 10);
        
        auto song = songs.at(1).song();
        ensure("title", song->title() == u("Skinny Love"));
        ensure("albumart", song->albumArtUrl() == "http://lh4.googleusercontent.com/yzO-wGxGgxOBSGhhWboYrJ0SHuVEP2yvnoPyta8WaHYwVRLg-gve4ZPgqse1IeA4ThSU8pib");
        ensure("artist", song->artist() == u("Bon Iver"));
        ensure("albumArtist", song->albumArtist() == u("Bon Iver"));
        ensure("album", song->album() == u("For Emma, Forever Ago"));
        ensure("albumId", song->albumId() == "Bayvujvcnydrhtz3beanjlz6fo4");
        ensure("artistId", song->artistId() == "Almf54cg3d2djde3cbziyla3uri");
        ensure("uniqueid", song->uniqueId() == "T3yiugpkex6p7jmlyymcmvzzjbm");
    }
    
    template<>
    template<>
    void testobject::test<2>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testSearchReply), session);
        auto artists = conceiver.artists();
        
        ensure("artistcount", artists.size() == 10);

        auto artist = artists.at(1);
        string testId = artist->uniqueId();
        ensure("uniqueid", testId == "A7gkcirwl4lpnnfgmti3unznhgq");
    }
    
    template<>
    template<>
    void testobject::test<3>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testSearchReply), session);
        auto albums = conceiver.albums();
        
        ensure("count", albums.size() == 10);
        
        auto album = albums.at(1);
        string testId = album->uniqueId();
        ensure("uniqueid", testId == "By5n6twtgnlepqcx4amdyau6nc4");
    }
    
    template<>
    template<>
    void testobject::test<4>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testAlbumFetch), session, static_cast<int>(GooglePlayConceiver::Hint::fetchAlbum));
        auto songs = conceiver.songs();
        
        ensure("songcount", songs.size() == 11);
        
        auto song = songs.at(1).song();
        ensure("title", song->title() == u("Orestis - Contact With Peace"));
        ensure("albumart", song->albumArtUrl() == "http://lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy");
        ensure("artist", song->artist() == u("Goa Gil"));
        ensure("albumArtist", song->albumArtist() == u("Goa Gil"));
        ensure("album", song->album() == u("Worldbridger"));
        ensure("uniqueid", song->uniqueId() == "Tajin3kv7ae5wtbirggpkotgkem");
    }
    
    template<>
    template<>
    void testobject::test<5>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testArtistFetch), session, static_cast<int>(GooglePlayConceiver::Hint::fetchArtist));
        auto songs = conceiver.songs();
        
        ensure("songcount", songs.size() == 16);
        
        auto song = songs.at(1).song();
        ensure("title", song->title() == u("Kulu - Virtual Revolution"));
        ensure("albumart", song->albumArtUrl() == "http://lh3.googleusercontent.com/d0e0BvhIIVNjqsASuNXOdBCy2O16hBMSJdIYWM58TtVzTWxTK2w4ICb7BVFYFppkFVN15sCy");
        ensure("artist", song->artist() == u("Goa Gil"));
        ensure("albumArtist", song->albumArtist() == u("Goa Gil"));
        ensure("album", song->album() == u("Worldbridger"));
        ensure("uniqueid", song->uniqueId() == "Tut2an427pk2tnuarp5hxriwhwy");
    }
    
    template<>
    template<>
    void testobject::test<6>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testArtistFetch), session, static_cast<int>(GooglePlayConceiver::Hint::fetchArtist));

        auto albums = conceiver.albums();
        ensure("albumcount", albums.size() == 1);
        
        auto album = albums.at(0);
        string testId = album->uniqueId();
        ensure("uniqueid", testId == "B6o7ehs7mx4akcgi3uiw4s3nehm");
    }
    
    template<>
    template<>
    void testobject::test<7>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testRadioList), session, static_cast<int>(GooglePlayConceiver::Hint::radioListUser));
        auto radioList = conceiver.playlists();
        
        ensure("count", radioList.size() == 7);
        ensure("name", radioList.at(2)->name() == u("Front Line Assembly"));
        auto songArray = radioList.at(3)->songArray();
        // we might be too fast!
        // ensure_equals("beforefetch", songArray->size(), 0);
        
        // trigger fetch
        songArray->size();
        
        // fetch in background thread
        milliseconds w(10);
        sleep_for(w);
        auto radioId = session->lastRadioId;
        ensure("radioId", radioId == "3ebade0f-34f4-3fe4-b5c5-4e4a1f6231b2");
        ensure("afterfetch", songArray->size() == 1);
    }
    
    
    template<>
    template<>
    void testobject::test<8>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testRadioList), session, static_cast<int>(GooglePlayConceiver::Hint::radioListUser));
        auto radioList = conceiver.songs();
        
        ensure("count", radioList.size() == 5);
        ensure("name", radioList.at(2).song()->title() == u("The Vulture"));
    }
    
    
    template<>
    template<>
    void testobject::test<9>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testRadioFetch), session, static_cast<int>(GooglePlayConceiver::Hint::fetchRadio));
        auto songs = conceiver.songs();
        
        ensure("count", songs.size() == 25);
        
        auto song = songs.at(0).song();
        ensure("title", song->title() == u("Voyage to India (Buddha Sunset Del Mar Vocal Mix)"));
        ensure("albumart", song->albumArtUrl() == "http://lh6.googleusercontent.com/YBQB_LVTj-hcn341sbtftPeMCzw1eWUupjut1lhtQCCA9xDtSJS9cr8LnPtGeh0FSiQ3VcYWHO8");
        ensure("artist", song->artist() == u("Ragi"));
        ensure("albumArtist", song->albumArtist() == u("Ragi"));
        ensure("album", song->album() == u("Voyage to India (Buddha Sunset Del Mar Vocal Mix)"));
        ensure("uniqueid", song->uniqueId() == "9d28df8f-9bb4-307c-9488-1ef6eabf5db3");

        
        auto radioList = conceiver.playlists();
        
        ensure("count", radioList.size() == 1);
        auto songArray = radioList.at(0)->songArray();
        
        // trigger fetch
        songArray->size();
        
        // fetch in background thread
        milliseconds w(10);
        sleep_for(w);
        ensure("radioId", session->lastRadioId == "9f6d57e1-a40f-3a1d-a7dd-54c3925e5c5a");
        ensure("afterfetch", songArray->size() == 1);
    }
    
    template<>
    template<>
    void testobject::test<10>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testCreatePlaylist), session, static_cast<int>(GooglePlayConceiver::Hint::createPlaylist));
        
        auto playlists = conceiver.playlists();
        ensure_equals("playlistcount", playlists.size(), 1);
        auto playlist = playlists.at(0);
        
        const auto playlistId = playlist->playlistId();
        ensure("id", playlistId == "34c9f85c-e0c8-47ba-9b67-2c1097f2b749");
        
        auto songs = conceiver.songs();
        
        ensure("songcount", songs.size() == 4);
        ensure("songid", songs.at(2).entryId() == "d27fa0ba-3136-36db-91a0-375b5111d475");
    }
    
    template<>
    template<>
    void testobject::test<11>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testAddToPlaylist), session, static_cast<int>(GooglePlayConceiver::Hint::addToPlaylist));
        
        
        auto songs = conceiver.songs();
        
        ensure("songcount", songs.size() == 4);
        ensure("songid", songs.at(3).entryId() == "9fcd32c4-eb6d-3bdf-9cf8-f0a2514157b8");
    }
    
    template<>
    template<>
    void testobject::test<12>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testSearchForConcreteAlbum), session);
        auto albums = conceiver.albums();
        
        ensure_equals("count", albums.size(), 1);
        
        auto album = albums.at(0);
        string testId = album->uniqueId();
        ensure("uniqueid", testId == "Baitatm2ksm6bgnpiji26t4rh2m");
    }
    
    template<>
    template<>
    void testobject::test<13>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testSearchForConcreteArtist), session);
        auto albums = conceiver.albums();
        
        ensure_equals("count", albums.size(), 3);
        
        auto album = albums.at(0);
        string testId = album->uniqueId();
        ensure("uniqueid1", testId == "Bczpp5y4sg4vtcltlgo7gsmju3e");
        
        album = albums.at(1);
        testId = album->uniqueId();
        ensure("uniqueid2", testId == "B5f2cdfwbpkgwylspz353yi3nxq");
        
        album = albums.at(2);
        testId = album->uniqueId();
        ensure("uniqueid3", testId == "Baitatm2ksm6bgnpiji26t4rh2m");
    }
    
    template<>
    template<>
    void testobject::test<14>()
    {
        shared_ptr<MockSession> session = shared_ptr<MockSession>(new MockSession());
        GooglePlayConceiver conceiver(parse(testAddToLibrary), session, static_cast<int>(GooglePlayConceiver::Hint::addToLibrary));
        
        auto songs = conceiver.songs();
        
        ensure("count", songs.size() == 3);
        
        auto songEntry = songs.at(1);
        ensure("songid", songEntry.entryId() == "5b16fe5f-9242-35ad-a69e-fc238a884232");
        ensure("allaccessid", songEntry.song()->uniqueId() == "T64eygqxved6qiozzc53esf5zzi");
    }
}