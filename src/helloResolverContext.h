
#ifndef USD_HELLO_RESOLVER_CONTEXT_H
#define USD_HELLO_RESOLVER_CONTEXT_H

#include <pxr/pxr.h>
#include <pxr/usd/ar/api.h>
#include <pxr/usd/ar/defineResolverContext.h>

#include <string>
#include <vector>
#include <map>

PXR_NAMESPACE_OPEN_SCOPE


class HelloResolverContext
{
public:
    /// Default construct a context with no search path.
    HelloResolverContext() = default;        

    /// Construct a context with the given \p searchPath.
    /// Elements in \p searchPath should be absolute paths. If they are not,
    /// they will be anchored to the current working directory.
    AR_API HelloResolverContext(const std::vector<std::string>& searchPath);

    AR_API void ToReplace(const std::string& oldStr, const std::string& newStr);

    const std::map<std::string, std::string>& GetStringsToReplace() const { return _oldAndNewStrings; }

    AR_API bool operator<(const HelloResolverContext& rhs) const;
    AR_API bool operator==(const HelloResolverContext& rhs) const;
    AR_API bool operator!=(const HelloResolverContext& rhs) const;

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
hash_value(const HelloResolverContext& context);

inline std::string 
ArGetDebugString(const HelloResolverContext& context)
{
    return context.GetAsString();
}

AR_DECLARE_RESOLVER_CONTEXT(HelloResolverContext);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // USD_HELLO_RESOLVER_CONTEXT_H
