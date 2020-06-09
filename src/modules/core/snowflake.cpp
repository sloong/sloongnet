
#include "snowflake.h"

using Sloong::snowflake;
using std::unique_ptr;
using std::make_unique;

unique_ptr<snowflake> Sloong::snowflake::Instance = make_unique<snowflake>();

