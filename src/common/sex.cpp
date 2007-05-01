/*sex.cpp*/

// TODO: let it use std::string

/* Original author unknown.  Presumably this is public domain by now.
 * If you are the original author or know the original author, please
 * contact <freebsd@spatula.net>
 *
 * Orphan code cleaned up a bit by Nick Johnson <freebsd@spatula.net>
 * Completely rewrote how word wrapping works and added -w flag.
 *
 * Changed a little bit for OpenLieroX.
 */

#include "defs.h"

static char     *faster[] = {
  "\"Let the games begin!\"",             "\"Sweet Jesus!\"",
  "\"Not that!\"",                        "\"At last!\"",
  "\"Land o' Goshen!\"",                  "\"Is that all?\"",
  "\"Cheese it, the cops!\"",             "\"I never dreamed it could be\"",
  "\"If I do, you won't respect me!\"",   "\"Now!\"",
  "\"Open sesame!\"",                     "\"EMR!\"",
  "\"Again!\"",                           "\"Faster!\"",
  "\"Harder!\"",                          "\"Help!\"",
  "\"Fuck me harder!\"",                  "\"Is it in yet?\"",
  "\"You aren't my father!\"",            "\"Doctor, that's not *my* shou\"",
  "\"No, no, do the goldfish!\"",         "\"Holy Batmobile, Batman!\"",
  "\"He's dead, he's dead!\"",            "\"Take me, Robert!\"",
  "\"I'm a Republican!\"",                "\"Put four fingers in!\"",
  "\"What a lover!\"",                    "\"Talk dirty, you pig!\"",
  "\"The ceiling needs painting,\"",      "\"Suck harder!\"",
  "\"The animals will hear!\"",           "\"Not in public!\"",
};

static char     *said[] = {
        "bellowed",             "yelped",               "croaked",
        "growled",              "panted",               "moaned",
        "grunted",              "laughed",              "warbled",
        "sighed",               "ejaculated",           "choked",
        "stammered",            "wheezed",              "squealed",
        "whimpered",            "salivated",            "tongued",
        "cried",                "screamed",             "yelled",
        "said",
};

static char     *the[] = {
        "the",
};

static char     *fadj[] = {
        "saucy",                "wanton",               "unfortunate",
        "lust-crazed",          "nine-year-old",        "bull-dyke",
        "bisexual",             "gorgeous",             "sweet",
        "nymphomaniacal",       "large-hipped",         "freckled",
        "forty-five year old",  "white-haired",         "large-boned",
        "saintly",              "blind",                "bearded",
        "blue-eyed",            "large tongued",        "friendly",
        "piano playing",        "ear licking",          "doe eyed",
        "sock sniffing",        "lesbian",              "hairy",
};


static char     *female[] = {
   "baggage",              "hussy",                "woman",
   "Duchess",              "female impersonator",  "nymphomaniac",
   "virgin",               "leather freak",        "home-coming queen",
   "defrocked nun",        "bisexual budgie",      "cheerleader",
   "office secretary",     "sexual deviate",       "DARPA contract monitor",
   "little matchgirl",     "ceremonial penguin",   "femme fatale",
   "bosses' daughter",     "construction worker",  "sausage abuser",
   "secretary",            "Congressman's page",   "grandmother",
   "penguin",              "German shepherd",      "stewardess",
   "waitress",             "prostitute",           "computer science group",
   "housewife",
};

static char     *asthe[] = {
        "as the",
};

static char     *madjec[] = {
   "thrashing",            "slurping",             "insatiable",
   "rabid",                "satanic",              "corpulent",
   "nose-grooming",        "tripe-fondling",       "dribbling",
   "spread-eagled",        "orally fixated",       "vile",
   "awesomely endowed",    "handsome",             "mush-brained",
   "tremendously hung",    "three-legged",         "pile-driving",
   "cross-dressing",       "gerbil buggering",     "bung-hole stuffing",
   "sphincter licking",    "hair-pie chewing",     "muff-diving",
   "clam shucking",        "egg-sucking",          "bicycle seat sniffing",
};

static char     *male[] = {
   "rakehell",             "hunchback",            "lecherous lickspittle",
   "archduke",             "midget",               "hired hand",
   "great Dane",           "stallion",             "donkey",
   "electric eel",         "paraplegic pothead",   "dirty old man",
   "faggot butler",        "friar",                "black-power advocate",
   "follicle fetishist",   "handsome priest",      "chicken flicker",
   "homosexual flamingo",  "ex-celibate",          "drug sucker",
   "ex-woman",             "construction worker",  "hair dresser",
   "dentist",              "judge",                "social worker",
};

static char     *diddled[] = {
   "diddled",              "devoured",             "fondled",
   "mouthed",              "tongued",              "lashed",
   "tweaked",              "violated",             "defiled",
   "irrigated",            "penetrated",           "ravished",
   "hammered",             "bit",                  "tongue slashed",
   "sucked",               "fucked",               "rubbed",
   "grudge fucked",        "masturbated with",     "slurped",
};

char *her[] = {
        "her",
};

static char     *titadj[] = {
        "alabaster",            "pink-tipped",          "creamy",
        "rosebud",              "moist",                "throbbing",
        "juicy",                "heaving",              "straining",
        "mammoth",              "succulent",            "quivering",
        "rosey",                "globular",             "varicose",
        "jiggling",             "bloody",               "tilted",
        "dribbling",            "oozing",               "firm",
        "pendulous",            "muscular",             "bovine",
};

static char     *knockers[] = {
        "globes",               "melons",               "mounds",
        "buds",                 "paps",                 "chubbies",
        "protuberances",        "treasures",            "buns",
        "bung",                 "vestibule",            "armpits",
        "tits",                 "knockers",             "elbows",
        "eyes",                 "hooters",              "jugs",
        "lungs",                "headlights",           "disk drives",
        "bumpers",              "knees",                "fried eggs",
        "buttocks",             "charlies",             "ear lobes",
        "bazooms",              "mammaries",
};

char *_and[] = {
        "and",
};

static char     *thrust[] = {
        "plunged",              "thrust",               "squeezed",
        "pounded",              "drove",                "eased",
        "slid",                 "hammered",             "squished",
        "crammed",              "slammed",              "reamed",
        "rammed",               "dipped",               "inserted",
        "plugged",              "augured",              "pushed",
        "ripped",               "forced",               "wrenched",
};

static char     *his[] = {
        "his",
};

static char     *dongadj[] = {
        "bursting",             "jutting",              "glistening",
        "Brobdingnagian",       "prodigious",           "purple",
        "searing",              "swollen",              "rigid",
        "rampaging",            "warty",                "steaming",
        "gorged",               "trunklike",            "foaming",
        "spouting",             "swinish",              "prosthetic",
        "blue veined",          "engorged",             "horse like",
        "throbbing",            "humongous",            "hole splitting",
        "serpentine",           "curved",               "steel encased",
        "glass encrusted",      "knobby",               "surgically altered",
        "metal tipped",         "open sored",           "rapidly dwindling",
        "swelling",             "miniscule",            "boney",
};

static char     *dong[] = {
   "intruder",             "prong",                "stump",
   "member",               "meat loaf",            "majesty",
   "bowsprit",             "earthmover",           "jackhammer",
   "ramrod",               "cod",                  "jabber",
   "gusher",               "poker",                "engine",
   "brownie",              "joy stick",            "plunger",
   "piston",               "tool",                 "manhood",
   "lollipop",             "kidney prodder",       "candlestick",
   "John Thomas",          "arm",                  "testicles",
   "balls",                "finger",               "foot",
   "tongue",               "dick",                 "one-eyed wonder worm",
   "canyon yodeler",       "middle leg",           "neck wrapper",
   "stick shift",          "dong",                 "Linda Lovelace choker",
};

static char     *intoher[] = {
        "into her",
};

static char     *twatadj[] = {
        "pulsing",              "hungry",               "hymeneal",
        "palpitating",          "gaping",               "slavering",
        "welcoming",            "glutted",              "gobbling",
        "cobwebby",             "ravenous",             "slurping",
        "glistening",           "dripping",             "scabiferous",
        "porous",               "soft-spoken",          "pink",
        "dusty",                "tight",                "odiferous",
        "moist",                "loose",                "scarred",
        "weapon-less",          "banana stuffed",       "tire tracked",
        "mouse nibbled",        "tightly tensed",       "oft traveled",
        "grateful",             "festering",
};

static char     *twat[] = {
        "swamp.",               "honeypot.",            "jam jar.",
        "butterbox.",           "furburger.",           "cherry pie.",
        "cush.",                "slot.",                "slit.",
        "cockpit.",             "damp.",                "furrow.",
        "sanctum sanctorum.",   "bearded clam.",        "continental divide.",
        "paradise valley.",     "red river valley.",    "slot machine.",
        "quim.",                "palace.",              "ass.",
        "rose bud.",            "throat.",              "eye socket.",
        "tenderness.",          "inner ear.",           "orifice.",
        "appendix scar.",       "wound.",               "navel.",
        "mouth.",               "nose.",                "cunt.",
};

struct table {
        char    **item;
        short   len;
};

typedef struct table    TABLE;
#define SZ(a)           sizeof(a) / sizeof(char *)

TABLE   list[] = {
        {faster,         SZ(faster)},     {said,           SZ(said)},
        {the,            SZ(the)},        {fadj,           SZ(fadj)},
        {female,         SZ(female)},     {asthe,          SZ(asthe)},
        {madjec,         SZ(madjec)},     {male,           SZ(male)},
        {diddled,        SZ(diddled)},    {her,            SZ(her)},
        {titadj,         SZ(titadj)},     {knockers,       SZ(knockers)},
        {_and,           SZ(_and)},        {thrust,         SZ(thrust)},
        {his,            SZ(his)},        {dongadj,        SZ(dongadj)},
        {dong,           SZ(dong)},       {intoher,        SZ(intoher)},
        {twatadj,        SZ(twatadj)},    {twat,           SZ(twat)},
        {(char **)NULL,  (short)NULL},
};

static short    lwidth;

const char* sex__cstring(short wraplen) {
	static char sextmp[2048];
   register TABLE  *ttp;
   register char   *cp;
   char* buffer = sextmp;
   int pos, lastword;
	lwidth = 0;
	
   pos = lastword = 0;
   for (ttp = list;ttp->item;++ttp,++lwidth) {
      for (cp = ttp->len > 1 ? ttp->item[GetRandomInt(ttp->len-1)] :
           *ttp->item;*cp;++cp,++lwidth) {
	buffer[pos] = *cp;
	if ((wraplen > 0) && (lwidth >= wraplen)) {
	    buffer[lastword] = '\n';
	    lwidth = pos - lastword;
	}
	if (isspace(*cp)) {
	    lastword = pos;
	} 
	pos++;
      }
      buffer[pos] = ' ';
      lastword = pos++;
   }
   buffer[pos] = '\0';

   return buffer;
}

std::string sex(short wraplen) {
	return sex__cstring(wraplen);
}
