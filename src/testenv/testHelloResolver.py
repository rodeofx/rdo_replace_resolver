from pxr import Ar
from pxr import Kind
from pxr import Sdf
from pxr import Usd
from pxr import Vt

from rdo import HelloResolver

import os
import unittest
import shutil


def _PrepAssets(rootDir, assemblyRepo, componentRepo):
    ''' Construct following structure

    a1_v1.usda     b1_v1.usda      c1_v1.usda
    /a  ---(R)---> /b   ---(R)---> /c
                                     x = 1                        
                   b1_v2.usda      c1_v2.usda
                     y = "b1_v2"     x = 2

    a1_v2.usda
    / customLayerData = replace string list
    /   ---(S)---> a1_v1.usda

    x and y attributes are used to check if the stage is 
    composed accordingly when our resolver replace versions.
    '''

    context = HelloResolver.HelloResolverContext([os.path.abspath(rootDir)])

    # Create c1_v1.usda
    c1_v1_relativePath = 'c1_v1.usda'
    _path = os.path.join(componentRepo, c1_v1_relativePath)
    stage = Usd.Stage.CreateNew(_path, context)
    prim = stage.DefinePrim('/c')
    Usd.ModelAPI(prim).SetKind(Kind.Tokens.component)
    stage.SetDefaultPrim(prim)
    xAttr = prim.CreateAttribute('x', Sdf.ValueTypeNames.Double, True)
    xAttr.Set(1.0)
    stage.Save()

    # Create c1_v2.usda
    c1_v2_relativePath = 'c1_v2.usda'
    _path = os.path.join(componentRepo, c1_v2_relativePath)
    stage = Usd.Stage.CreateNew(_path, context)
    stage.GetRootLayer().subLayerPaths = [c1_v1_relativePath]
    prim = stage.GetPrimAtPath('/c')
    stage.SetDefaultPrim(prim)
    xAttr = prim.GetAttribute('x')
    xAttr.Set(2.0)
    stage.Save()

    # Create b1_v1.usda
    b1_v1_relativePath = 'b1_v1.usda'
    _path = os.path.join(assemblyRepo, b1_v1_relativePath)
    stage = Usd.Stage.CreateNew(_path, context)
    prim = stage.DefinePrim('/b')
    Usd.ModelAPI(prim).SetKind(Kind.Tokens.assembly)
    stage.SetDefaultPrim(prim)

    prim.GetReferences().AddReference('component/' + c1_v1_relativePath)
    stage.Save()

    # Create b1_v2.usda
    b1_v2_relativePath = 'b1_v2.usda'
    _path = os.path.join(assemblyRepo, b1_v2_relativePath)
    stage = Usd.Stage.CreateNew(_path, context)
    stage.GetRootLayer().subLayerPaths = [b1_v1_relativePath]
    prim = stage.GetPrimAtPath('/b')
    stage.SetDefaultPrim(prim)
    yAttr = prim.CreateAttribute('y', Sdf.ValueTypeNames.String, True)
    yAttr.Set('b1_v2')

    stage.Save()

    # Create a1_v1.usda
    a1_v1_relativePath = 'a1_v1.usda'
    _path = os.path.join(assemblyRepo, a1_v1_relativePath)
    stage = Usd.Stage.CreateNew(_path, context)
    prim = stage.DefinePrim('/a')
    Usd.ModelAPI(prim).SetKind(Kind.Tokens.assembly)
    stage.SetDefaultPrim(prim)

    prim.GetReferences().AddReference("assembly/" + b1_v1_relativePath)
    stage.Save()

    # Create a1_v2.usda
    a1_v2_relativePath = 'a1_v2.usda'
    _path = os.path.join(assemblyRepo, a1_v2_relativePath)
    stage = Usd.Stage.CreateNew(_path, context)
    pair1 = ['component/c1_v1.usda', 'component/c1_v2.usda']
    pair2 = ['assembly/b1_v1.usda', 'assembly/b1_v2.usda']
    stage.SetMetadata('customLayerData', {HelloResolver.Tokens.replacePairs: Vt.StringArray(pair1 + pair2)})
    stage.GetRootLayer().subLayerPaths = [a1_v1_relativePath]
    stage.Save()


class TestHelloResolver(unittest.TestCase):

    rootDir = 'testHelloResolver'
    componentRepo = os.path.join(rootDir, "component")
    assemblyRepo = os.path.join(rootDir, "assembly")

    def assertPathsEqual(self, path1, path2):
        # Flip backslashes to forward slashes to accommodate platform
        # differences. We don't use os.path.normpath since that might
        # fix up other differences we'd want to catch in these tests.
        self.assertEqual(path1.replace("\\", "/"), path2.replace("\\", "/"))

    @classmethod
    def setUpClass(cls):
        # Force Ar to use the hello resolver implementation.
        Ar.SetPreferredResolver('HelloResolver')

        # Verify that the underlying resolver is a HelloResolver.
        assert(isinstance(Ar.GetUnderlyingResolver(), HelloResolver.HelloResolver))

        # Cleanup files from previous test
        dirToRemove = os.path.abspath(TestHelloResolver.rootDir)
        if os.path.isdir(dirToRemove):
            shutil.rmtree(dirToRemove)

        _PrepAssets(TestHelloResolver.rootDir,
            TestHelloResolver.assemblyRepo,
            TestHelloResolver.componentRepo)

    def test_ResolveWithContext(self):
        context = HelloResolver.HelloResolverContext([os.path.abspath(TestHelloResolver.rootDir)])

        context.AddReplacePair('component/c1_v1.usda', 'component/c1_v2.usda')
        context.AddReplacePair('assembly/b1_v1.usda', 'assembly/b1_v2.usda')

        with Ar.ResolverContextBinder(context):
            resolver = Ar.GetResolver()

            expectedPath = os.path.join(TestHelloResolver.rootDir, 'component', 'c1_v2.usda')
            self.assertEqual(
                resolver.Resolve('component/c1_v1.usda'),
                os.path.abspath(expectedPath))

            expectedPath = os.path.join(TestHelloResolver.rootDir, 'assembly', 'b1_v2.usda')
            self.assertEqual(
                resolver.Resolve('assembly/b1_v1.usda'),
                os.path.abspath(expectedPath))

    def test_ResolveFromStageOneLevel(self):
        ''' Replace reference to c1_v1 by c1_v2 and open stage to check x value '''
        context = HelloResolver.HelloResolverContext([os.path.abspath(TestHelloResolver.rootDir)])
        context.AddReplacePair('component/c1_v1.usda', 'component/c1_v2.usda')

        filePath = os.path.join(TestHelloResolver.assemblyRepo, 'b1_v1.usda')
        stage = Usd.Stage.Open(filePath, pathResolverContext=context)

        prim = stage.GetPrimAtPath('/b')
        xAttr = prim.GetAttribute('x')
        
        self.assertEqual(xAttr.Get(), 2.0)

    def test_ResolveFromStageTwoLevels(self):
        ''' 
        In a1_v1, replace reference to b1_v1 by b1_v2
        In b1_v2, replace reference to c1_v1 by c1_v2

        Check x and y values
        '''
        context = HelloResolver.HelloResolverContext([os.path.abspath(TestHelloResolver.rootDir)])
        context.AddReplacePair('component/c1_v1.usda', 'component/c1_v2.usda')
        context.AddReplacePair('assembly/b1_v1.usda', 'assembly/b1_v2.usda')

        filePath = os.path.join(TestHelloResolver.assemblyRepo, 'a1_v1.usda')
        stage = Usd.Stage.Open(filePath, pathResolverContext=context)

        prim = stage.GetPrimAtPath('/a')
        yAttr = prim.GetAttribute('y')
        
        self.assertEqual(yAttr.Get(), 'b1_v2')

    def test_ResolveFromMetaData(self):
        ''' 
        In a1_v2, replace reference to b1_v1 by b1_v2
        In b1_v2, replace reference to c1_v1 by c1_v2

        Check x and y values
        '''
        filePath = os.path.join(TestHelloResolver.assemblyRepo, 'a1_v2.usda')
        os.environ['PXR_AR_DEFAULT_SEARCH_PATH'] = os.path.abspath(TestHelloResolver.rootDir)
        stage = Usd.Stage.Open(filePath)

        prim = stage.GetPrimAtPath('/a')
        self.assertTrue(prim)

        xAttr = prim.GetAttribute('x')
        self.assertTrue(xAttr)  
        self.assertEqual(xAttr.Get(), 2.0)

        yAttr = prim.GetAttribute('y')
        self.assertTrue(yAttr)  
        self.assertEqual(yAttr.Get(), 'b1_v2')


if __name__ == '__main__':
    unittest.main()

