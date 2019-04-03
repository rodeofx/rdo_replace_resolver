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
    HelloResolverContext context({"/tmp/local", "/tmp/installed"});
    context.ToReplace("o_v1", "o_v2");

    std::cout << context.GetAsString() <<  std::endl;
    {
        // Bind the context object:
        ArResolverContextBinder binder(context);

        std::string path = "foo_v1.usda";
        std::string resolvedPath = ArGetResolver().Resolve(path);
        std::cout << path << " resolved to: " << resolvedPath << '\n';
    }

    return 0;
}
