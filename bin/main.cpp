#include "helloResolverContext.h"

#include <pxr/usd/ar/defaultResolverContext.h>

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/ar/resolver.h>
#include <pxr/usd/ar/resolverContextBinder.h>

#include <iostream>

PXR_NAMESPACE_USING_DIRECTIVE


int main(int argc, const char** argv)
{
    ArSetPreferredResolver("HelloResolver");
    HelloResolverContext ctx({"/tmp/local", "/tmp/installed"});
    {
        // Bind the context object:
        ArResolverContextBinder binder(ctx);

        // std::string resolvedPath = ArGetUnderlyingResolver().Resolve("foo.usda");
        // std::cout << resolvedPath << '\n';
    }

    HelloResolverContext ctx2({"FOO", "BAR"});
    ArResolverContextBinder binder2(ctx2);

    return 0;
}
