// Copyright 2019 Rodeo FX.  All rights reserved.

#include "boost_include_wrapper.h"

#include BOOST_INCLUDE(python/class.hpp)
#include "tokens.h"

PXR_NAMESPACE_USING_DIRECTIVE

namespace {

// Helper to return a static token as a string.  We wrap tokens as Python
// strings and for some reason simply wrapping the token using def_readonly
// bypasses to-Python conversion, leading to the error that there's no
// Python type for the C++ TfToken type.  So we wrap this functor instead.
class _WrapStaticToken {
public:
    _WrapStaticToken(const TfToken* token) : _token(token) { }

    std::string operator()() const
    {
        return _token->GetString();
    }

private:
    const TfToken* _token;
};

template <typename T>
void
_AddToken(T& cls, const char* name, const TfToken& token)
{
    cls.add_static_property(name,
                            BOOST_NAMESPACE::python::make_function(
                                _WrapStaticToken(&token),
                                BOOST_NAMESPACE::python::return_value_policy<
                                    BOOST_NAMESPACE::python::return_by_value>(),
                                BOOST_NAMESPACE::mpl::vector1<std::string>()));
}

} // anonymous

void wrapReplaceResolverTokens()
{
    BOOST_NAMESPACE::python::class_<ReplaceResolverTokensType, BOOST_NAMESPACE::noncopyable>
        cls("Tokens", BOOST_NAMESPACE::python::no_init);
    _AddToken(cls, "replacePairs", ReplaceResolverTokens->replacePairs);
    _AddToken(cls, "replaceFileName", ReplaceResolverTokens->replaceFileName);
}
