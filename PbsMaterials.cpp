
#include "GraphicsSystem.h"
#include "PbsMaterialsGameState.h"
#include "SdlInputHandler.h"

#include "OgreTimer.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreConfigFile.h"
#include "OgreTextureManager.h"
#include "OgreMaterialManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreManualObject2.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"
#include "OgreRenderTarget.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"
#include "OgreHlmsUnlitDatablock.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"

//Declares WinMain / main
#include "MainEntryPointHelper.h"

using namespace Demo;

namespace Demo
{
    class PbsMaterialsGraphicsSystem : public GraphicsSystem
    {
        virtual Ogre::CompositorWorkspace* setupCompositor()
        {
            Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();
            return compositorManager->addWorkspace( mSceneManager, mRenderWindow, mCamera,
                                                    "PbsMaterialsWorkspace", true );
        }

        virtual void setupResources(void)
        {
            GraphicsSystem::setupResources();

            Ogre::ConfigFile cf;
            cf.load(mResourcePath + "resources2.cfg");

            Ogre::String dataFolder = cf.getSetting( "DoNotUseAsResource", "Hlms", "" );

            if( dataFolder.empty() )
                dataFolder = "./";
            else if( *(dataFolder.end() - 1) != '/' )
                dataFolder += "/";

            dataFolder += "2.0/scripts/materials/PbsMaterials";

            addResourceLocation( dataFolder, "FileSystem", "General" );
        }

    public:
        PbsMaterialsGraphicsSystem( GameState *gameState ) :
            GraphicsSystem( gameState )
        {
        }


		Ogre::CompositorWorkspace*	m_CustomRenderTarget;
		Ogre::TexturePtr			m_CustomRenderTexture;
		Ogre::Camera*				m_CustomRTTCamera;
		void SetUpCustomContent()
		{
			Ogre::String CustomCameraName = "TestRenderTargetCamera";
			Ogre::String CustomTextureName = "TestRenderTargetTexture";
			Ogre::String CustomMaterialName = "CustomRenderTargetMaterial";
			Ogre::String CustomWorkSpaceName = "TestCustomRenderTargetWorkSpaceName";

			m_CustomRTTCamera = mSceneManager->createCamera(CustomCameraName);
			m_CustomRTTCamera->setPosition(0, 30, 0);
			m_CustomRTTCamera->lookAt(0, 0, 0);
			m_CustomRTTCamera->setFarClipDistance(1000);
			m_CustomRTTCamera->setNearClipDistance(0.1);

			m_CustomRenderTexture = Ogre::TextureManager::getSingleton().createManual(CustomTextureName, 
				Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 
				512, 512, 1, Ogre::PF_A8B8G8R8, Ogre::TU_RENDERTARGET);
			m_CustomRenderTexture->load();

			Ogre::RenderTexture* rtt = m_CustomRenderTexture->getBuffer(0)->getRenderTarget();


			Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

			const Ogre::IdString workspaceName(CustomWorkSpaceName);
			if( !compositorManager->hasWorkspaceDefinition( workspaceName ) )
			{
				compositorManager->createBasicWorkspaceDef( workspaceName, mBackgroundColour,
					Ogre::IdString() );
			}

			m_CustomRenderTarget = compositorManager->addWorkspace( mSceneManager, (Ogre::RenderTarget*)rtt, m_CustomRTTCamera,
					workspaceName, false );
			//m_CustomRenderTarget->setEnabled(false);			// not auto update
#if 0
			// create manual object
			Ogre::MaterialPtr CustomMaterial = Ogre::MaterialManager::getSingleton().create(CustomMaterialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			CustomMaterial->getTechnique(0)->removeAllPasses();
			Ogre::Pass* pass = CustomMaterial->getTechnique(0)->createPass();
			Ogre::TextureUnitState* tu = pass->createTextureUnitState();
			tu->setTextureName(CustomTextureName);
#endif
			Ogre::HlmsManager *hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
			Ogre::Hlms *hlms = hlmsManager->getHlms( Ogre::HLMS_UNLIT );
			Ogre::HlmsDatablock *datablock = hlms->createDatablock( CustomMaterialName, CustomMaterialName,
				Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec() );
			Ogre::HlmsUnlitDatablock *unlitDb = static_cast<Ogre::HlmsUnlitDatablock*>( datablock );
			unlitDb->setTexture( 0, 0, m_CustomRenderTexture );
			
#if 1
			Ogre::ManualObject* CustomManualObject = mSceneManager->createManualObject();
			CustomManualObject->begin(CustomMaterialName);
			CustomManualObject->position(-100, -100, -100);
			CustomManualObject->textureCoord(0, 0);
			CustomManualObject->position(100, -100, -100);
			CustomManualObject->textureCoord(1, 0);
			CustomManualObject->position(100, 100, -100);
			CustomManualObject->textureCoord(1, 1);
			CustomManualObject->position(-100, 100, -100);
			CustomManualObject->textureCoord(0, 1);
			CustomManualObject->quad(0, 1, 2, 3);
			CustomManualObject->end();
//			CustomManualObject->setDatablock(0, CustomMaterialName);

			Ogre::SceneNode *sceneNodeLines = mSceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
				createChildSceneNode( Ogre::SCENE_DYNAMIC );
			sceneNodeLines->attachObject(CustomManualObject);
			sceneNodeLines->scale(0.4f, 0.4f, 0.4f);
			sceneNodeLines->translate(0.0f, 0.0f, 0.0f, Ogre::SceneNode::TS_WORLD);
#endif
		}
		void CustomUpdate()
		{
			mSceneManager->updateSceneGraph();
			m_CustomRenderTarget->_validateFinalTarget();
			mRoot->getRenderSystem()->_beginFrameOnce();
			m_CustomRenderTarget->_beginUpdate(false);
			m_CustomRenderTarget->_update();
			m_CustomRenderTarget->_endUpdate(false);
			//m_CustomRenderTarget->_swapFinalTarget();
			mSceneManager->_frameEnded();
			Ogre::HlmsManager *hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
			for( size_t i=0; i < Ogre::HLMS_MAX; ++i )
			{
				Ogre::Hlms *hlms = hlmsManager->getHlms( static_cast<Ogre::HlmsTypes>( i ) );
				if( hlms )
					hlms->frameEnded();
			}
			mRoot->getRenderSystem()->_update();
		}
    };
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int mainApp()
#endif
{
    PbsMaterialsGameState pbsMaterialsGameState(
        "Shows how to use the PBS material system. There's nothing really fancy,\n"
        "it's just programmer art. The PBS materials can be setup from script or\n"
        "code. This sample does both. At the time being, not all settings from the\n"
        "PBS implementation can be tweaked with scripts. See PbsDatablock::PbsDatablock\n"
        "constructor documentation. Also see the Hlms section of the porting guide in\n"
        "the Docs/2.0 folder.\n"
        "\n"
        "The sphere palette shows what happens when tweaking the roughness around the\n"
        "X axis; and the fresnel term around the Z axis.\n"
        "The scene is being lit by a white directional light (3-split PSSM) and two spot\n"
        "lights, one of warm colour, one cold. Both are also shadowed."
        "\n"
        "Of all the features supported by the PBS implementation, perhaps the hardest to\n"
        "to understand is the Detail Weight Map. It allows you to 'paint' the detail maps\n"
        "over the mesh, by controlling weight of each of the 4 maps via the RGBA channels\n"
        "of the weight map. 'R' controls the detail map 0, 'G' the detail map 1,\n"
        "'B' the detail map 2, and 'A' the detail map 3.\n"
        "\n"
        "This sample depends on the media files:\n"
        "   * Samples/Media/2.0/scripts/Compositors/PbsMaterials.compositor\n"
        "   * Samples/Media/2.0/materials/PbsMaterials/PbsMaterials.material\n"
        "\n"
        "Known issues:\n"
        " * Non shadow casting point & spot lights require Forward3D to be enabled (on desktop).\n"
        "   This is by design (more implementations will come: Forward+ & Deferred; for now the\n"
        "   only one working is F3D).\n"
        " * Shadow casting point lights don't work or work poorly. (feature not implemented yet)\n"
        " * If PSSM shadow casting enabled, the system requires at least one shadow-casting\n"
        "   directional light (bug)\n"
        " * Mobile version only supports forward lighting.\n"
        "\n"
        "LEGAL: Uses Saint Peter's Basilica (C) by Emil Persson under CC Attrib 3.0 Unported\n"
        "See Samples/Media/materials/textures/Cubemaps/License.txt for more information.");
    PbsMaterialsGraphicsSystem graphicsSystem( &pbsMaterialsGameState );

    pbsMaterialsGameState._notifyGraphicsSystem( &graphicsSystem );

    graphicsSystem.initialize( "PBS Materials Sample" );

    if( graphicsSystem.getQuit() )
    {
        graphicsSystem.deinitialize();
        return 0; //User cancelled config
    }

    Ogre::RenderWindow *renderWindow = graphicsSystem.getRenderWindow();

    graphicsSystem.createScene01();
    graphicsSystem.createScene02();

	graphicsSystem.SetUpCustomContent();			// create my custom contents

    //Do this after creating the scene for easier the debugging (the mouse doesn't hide itself)
    SdlInputHandler *inputHandler = graphicsSystem.getInputHandler();
    inputHandler->setGrabMousePointer( true );
    inputHandler->setMouseVisible( false );

    Ogre::Timer timer;
    unsigned long startTime = timer.getMicroseconds();

    double timeSinceLast = 1.0 / 60.0;

    while( !graphicsSystem.getQuit() )
    {
		graphicsSystem.CustomUpdate();		// update my custom rtts
        graphicsSystem.beginFrameParallel();
        graphicsSystem.update( static_cast<float>( timeSinceLast ) );
        graphicsSystem.finishFrameParallel();
        graphicsSystem.finishFrame();

        if( !renderWindow->isVisible() )
        {
            //Don't burn CPU cycles unnecessary when we're minimized.
            Ogre::Threads::Sleep( 500 );
        }

        unsigned long endTime = timer.getMicroseconds();
        timeSinceLast = (endTime - startTime) / 1000000.0;
        timeSinceLast = std::min( 1.0, timeSinceLast ); //Prevent from going haywire.
        startTime = endTime;
    }

    graphicsSystem.destroyScene();
    graphicsSystem.deinitialize();

    return 0;
}
