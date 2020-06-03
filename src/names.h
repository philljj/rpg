#if !defined(NAMES_H)
#define NAMES_H

#define NUM_MOB_SUB_TYPES (4)
// Animals.
static const char * flying_list[] = {
    "bat", "gull", "owl", "buzzard"
};

static const char * dog_list[] = {
    "starving wolf", "coyote", "bloodhound", "dire wolf"
};

static const char * cat_list[] = {
    "starving panther", "mountain lion", "shadowcat", "tiger"
};

static const char * boar_list[] = {
    "boar", "tusked boar", "pig", "great boar"
};

static const char * bear_list[] = {
    "brown bear", "black bear", "polar bear", "dire bear"
};

// Dragons.
static const char * whelp_list[] = {
    "whelp", "forest whelp", "whelpling", "searing whelp"
};

static const char * forest_dragon_list[] = {
    "verdant dragon", "forest dragon", "moss drake", "green wyrm"
};

static const char * sand_dragon_list[] = {
    "sand serpent", "desert dragon", "yellow drake", "sand wyrm"
};

static const char * water_dragon_list[] = {
    "water drake", "lake serpent", "blue drake", "deep ocean wyrm"
};

static const char * fire_dragon_list[] = {
    "searing drake", "ember serpent", "inferno dragon", "magma wyrm"
};

#define MAX_PREFIX (116)
static const char * prefix_list[MAX_PREFIX] = {
    "ab", "ae", "ag", "am", "an", "ba", "be", "bi", "bo", "bu",
    "ca", "ce", "ci", "co", "cu", "da", "de", "di", "do", "du",
    "fa", "fe", "fi", "fo", "fu", "ga", "ge", "gi", "go", "gu",
    "ha", "he", "hi", "ho", "hu", "ma", "me", "mi", "mo", "mu",
    "sa", "se", "si", "so", "su", "ta", "te", "ti", "to", "tu",//50
    "ag", "bag", "cag", "dag", "mag",
    "al", "el", "il", "ol", "ul",
    "isil", "ithil", "igil", "iril", "imil",
    "aeon", "feon", "heon", "leon", "theon",
    "ainar", "finar", "minar", "thinar", "sinar",
    "mith", "minith", "minas", "milith", "mae",
    "gala", "galad", "gal", "galat", "galag",
    "bala", "balad", "bal", "balat", "balag",
    "rha", "rhe", "rhi", "rho", "rhith",
    "cele", "celem", "curu", "cara", "cura",//100
    "ind", "im", "idril", "inglor", "irime",
    "tha", "the", "tho", "thi", "thu",
    "tham", "than", "thath", "thon", "thoth",
    "thal"
};

#define MAX_SUFFIX (105)
static const char * suffix_list[MAX_SUFFIX] = {
    "ab", "ae", "ag", "am", "an", "ba", "be", "bi", "bo", "bu",
    "ca", "ce", "ci", "co", "cu", "da", "de", "di", "do", "du",
    "fa", "fe", "fi", "fo", "fu", "ga", "ge", "gi", "go", "gu",
    "ha", "he", "hi", "ho", "hu", "ma", "me", "mi", "mo", "mu",
    "sa", "se", "si", "so", "su", "ta", "te", "ti", "to", "tu",//50
    "amm", "ath", "ass", "agg", "all",
    "emm", "eth", "ess", "egg", "ell",
    "imm", "ith", "iss", "igg", "ill",
    "omm", "oth", "oss", "ogg", "oll",
    "umm", "uth", "uss", "ugg", "ull",
    "dur", "bur", "gur", "thur", "nur",
    "endil", "andil", "indil", "ondil", "undil",
    "thig", "thim", "thin", "thir", "this",
    "ain", "din", "fin", "gin", "lin",
    "aith", "fith", "thith", "nith", "sith",//100
    "aeth", "feth", "theth", "neth", "seth"
};

#endif /* if !define(NAMES_H) */
