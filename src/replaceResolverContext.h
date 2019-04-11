// Copyright 2019 Rodeo FX.  All rights reserved.
#ifndef USD_REPLACE_RESOLVER_CONTEXT_H
#define USD_REPLACE_RESOLVER_CONTEXT_H

#include <pxr/pxr.h>
#include <pxr/usd/ar/api.h>
#include <pxr/usd/ar/defineResolverContext.h>

#include <string>
#include <vector>
#include <map>

PXR_NAMESPACE_OPEN_SCOPE


class ReplaceResolverContext
{
public:
    /// Default construct a context with no search path.
    ReplaceResolverContext() = default;        

    /// Construct a context with the given \p searchPath.
    /// Elements in \p searchPath should be absolute paths. If they are not,
    /// they will be anchored to the current working directory.
    AR_API ReplaceResolverContext(const std::vector<std::string>& searchPath);

    AR_API void AddReplacePair(const std::string& oldStr, const std::string& newStr);

    const std::map<std::string, std::string>& GetReplaceMap() const { return _oldAndNewStrings; }

    AR_API bool operator<(const ReplaceResolverContext& rhs) const;
    AR_API bool operator==(const ReplaceResolverContext& rhs) const;
    AR_API bool operator!=(const ReplaceResolverContext& rhs) const;

    /// Return this context's search path.
    const std::vector<std::string>& GetSearchPath() const
    {
        return _searchPath;
    }

    /// Return a string representation of this context for debugging.
    AR_API std::string GetAsString() const;

private:
    std::vector<std::string> _searchPath;
    std::map<std::string, std::string> _oldAndNewStrings;
};

AR_API size_t 
hash_value(const ReplaceResolverContext& context);

inline std::string 
ArGetDebugString(const ReplaceResolverContext& context)
{
    return context.GetAsString();
}

AR_DECLARE_RESOLVER_CONTEXT(ReplaceResolverContext);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // USD_REPLACE_RESOLVER_CONTEXT_H
