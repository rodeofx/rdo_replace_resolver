// Copyright 2019 Rodeo FX.  All rights reserved.
#include "tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

ReplaceResolverTokensType::ReplaceResolverTokensType() :
    replacePairs("replacePairs", TfToken::Immortal),
    replaceFileName("replace.json", TfToken::Immortal),
    allTokens({
        replacePairs
    })
{
}

TfStaticData<ReplaceResolverTokensType> ReplaceResolverTokens;

PXR_NAMESPACE_CLOSE_SCOPE

