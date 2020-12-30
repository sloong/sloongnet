#include "result.h"

using Sloong::CResult;
using Sloong::TResult;

CResult Sloong::CResult::Succeed = CResult(ResultType::Succeed);
CResult Sloong::CResult::Invalid = CResult(ResultType::Invalid);
CResult Sloong::CResult::Ignore = CResult(ResultType::Ignore);
CResult Sloong::CResult::Retry = CResult(ResultType::Retry);
