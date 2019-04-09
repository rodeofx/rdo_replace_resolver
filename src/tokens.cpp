#include "tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

HelloResolverTokensType::HelloResolverTokensType() :
    replacePairs("replacePairs", TfToken::Immortal),
    replaceFileName("replace.json", TfToken::Immortal),
    allTokens({
        replacePairs
    })
{
}

TfStaticData<HelloResolverTokensType> HelloResolverTokens;

PXR_NAMESPACE_CLOSE_SCOPE

