#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/Utilities.h"
#include "cinder/Matrix33.h"
#include "cinder/Matrix44.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Vector.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// TODO: add adaptive LOD via tessellation shaders
// TODO: add multi-texturing (bump mapping)
// TODO: add vertex attributes (tangent spaces)
// TODO: add multi-pass rendering (depth of field?)

class TessellationSampleApp : public AppNative {
  public:
	
	void setup();
	void mouseMove( MouseEvent event );
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	
	void resize();
	
	void keyDown( KeyEvent event );
	void update();
	void draw();
	
  private:
	gl::VboMeshRef createIcosahedron();
	
	gl::BatchRef		mBatch;
	gl::VboMeshRef		mMesh;
	gl::GlslProgRef		mGlsl;
	
	Vec2f mMousePos;
	MayaCamUI mMayaCam;
	Matrix33f mNormalMatrix;
	
	bool mDrawWireframe;
	int32_t mTessellationInner;
	int32_t mTessellationOuter;
	params::InterfaceGlRef	mParams;
};

void TessellationSampleApp::setup()
{
	// setup shader
	try {
//		mGlsl = gl::GlslProg::create( gl::GlslProg::Format()
//									 .vertex( loadAsset( "shaders/pipeline_vert.glsl" ) )
//									 .tessellationCtrl( loadAsset( "shaders/pipeline_tess_c.glsl" ) )
//									 .tessellationEval( loadAsset( "shaders/pipeline_tess_e.glsl" ) )
//									 .geometry( loadAsset( "shaders/pipeline_geo.glsl" ) )
//									 .fragment( loadAsset( "shaders/pipeline_frag.glsl" ) ) );
		mGlsl = gl::GlslProg::create( loadAsset( "shaders/pipeline_vert.glsl" ),
									  loadAsset( "shaders/pipeline_frag.glsl" ),
									  loadAsset( "shaders/pipeline_geo.glsl" ),
									  loadAsset( "shaders/pipeline_tess_e.glsl" ),
									  loadAsset( "shaders/pipeline_tess_c.glsl" ) );
	}
	catch( gl::GlslProgCompileExc ex ) {
		cout << ex.what() << endl;
		shutdown();
	}
	
	mDrawWireframe = false;
	
	// initialize uniform values
	mTessellationInner = mTessellationOuter = 3;
	mDrawWireframe = true;
	
	// generate vbo mesh
	mMesh = createIcosahedron();
	
	// create batch for mesh and shader
	map<geom::Attrib,string> attributeMap;
	attributeMap.insert( pair<geom::Attrib,string>( geom::Attrib::POSITION, "Position" ) );
	mBatch = gl::Batch::create(mMesh, mGlsl, attributeMap);
	
	// create camera to view tessellation geometry
	CameraPersp cam;
	cam.setEyePoint( Vec3f(7.0f, 7.0f, 7.0f) );
	cam.setCenterOfInterestPoint( Vec3f::zero() );
	cam.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 1000.0f );
	mMayaCam.setCurrentCam( cam );
	
	// setup parameters
	mParams = params::InterfaceGl::create( "Settings", Vec2i( 200, 200 ) );
	mParams->addParam( "Wireframe", &mDrawWireframe );
	mParams->addParam( "Tessellation inner level", &mTessellationInner, "min=1" );
	mParams->addParam( "Tessellation outer level", &mTessellationOuter, "min=1" );
}

void TessellationSampleApp::mouseMove( MouseEvent event )
{
	// keep track of the mouse
	mMousePos = event.getPos();
}

void TessellationSampleApp::mouseDown( MouseEvent event )
{
	// let the camera handle the interaction
	mMayaCam.mouseDown( event.getPos() );
}

void TessellationSampleApp::mouseDrag( MouseEvent event )
{
	// keep track of the mouse
	mMousePos = event.getPos();
	
	// let the camera handle the interaction
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void TessellationSampleApp::resize()
{
	// adjust aspect ratio
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
}

void TessellationSampleApp::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_UP ){
		mTessellationInner++;
	}
	else if( event.getCode() == KeyEvent::KEY_DOWN ){
		if (mTessellationInner > 1)
			mTessellationInner--;
	}
	else if( event.getCode() == KeyEvent::KEY_RIGHT ){
		mTessellationOuter++;
	}
	else if( event.getCode() == KeyEvent::KEY_LEFT ){
		if (mTessellationOuter > 1)
			mTessellationOuter--;
	}
	else if( event.getCode() == KeyEvent::KEY_f ){
		setFullScreen( !isFullScreen() );
	}
}

gl::VboMeshRef TessellationSampleApp::createIcosahedron()
{
	const uint32_t faces[] = {
		2, 1, 0,
		3, 2, 0,
		4, 3, 0,
		5, 4, 0,
		1, 5, 0,
		
		11, 6,	7,
		11, 7,	8,
		11, 8,	9,
		11, 9,	10,
		11, 10, 6,
		
		1, 2, 6,
		2, 3, 7,
		3, 4, 8,
		4, 5, 9,
		5, 1, 10,
		
		2,	7, 6,
		3,	8, 7,
		4,	9, 8,
		5, 10, 9,
		1,	6, 10
	};
	
	const float verts[] = {
		0.000f,	 0.000f,  1.000f,
		0.894f,	 0.000f,  0.447f,
		0.276f,	 0.851f,  0.447f,
		-0.724f,  0.526f,  0.447f,
		-0.724f, -0.526f,  0.447f,
		0.276f, -0.851f,  0.447f,
		0.724f,	 0.526f, -0.447f,
		-0.276f,  0.851f, -0.447f,
		-0.894f,  0.000f, -0.447f,
		-0.276f, -0.851f, -0.447f,
		0.724f, -0.526f, -0.447f,
		0.000f,	 0.000f, -1.000f
	};
	
	gl::VboRef indexVbo = gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, sizeof(faces), faces );
	gl::VboRef vertVbo = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(verts), verts );

	GLsizei stride = 3 * sizeof(float);
	geom::BufferLayout layout;
	layout.append( geom::Attrib::POSITION, 3, stride, 0 );
	vector<pair<geom::BufferLayout,gl::VboRef>> vertexArrayBuffers;
	vertexArrayBuffers.push_back( pair<geom::BufferLayout,gl::VboRef>( layout, vertVbo ) );
	gl::VboMeshRef vboMesh = gl::VboMesh::create( sizeof(verts), GL_PATCHES, vertexArrayBuffers, sizeof(faces), GL_UNSIGNED_INT, indexVbo );
	
	// why is this not needed?
//	map<geom::Attrib,string> attributeMap;
//	attributeMap.insert( pair<geom::Attrib,string>( geom::Attrib::POSITION, "Position" ) );
//	mIcosahedronMesh->buildVao( mGlsl, attributeMap );
	
	gl::checkError();
	
	return vboMesh;
}

void TessellationSampleApp::update()
{
	mNormalMatrix = mMayaCam.getCamera().getViewMatrix().subMatrix33(0,0);
	mNormalMatrix.transpose();
}

void TessellationSampleApp::draw()
{
	// clear out the window with black
	gl::clear( ColorA(0.7f, 0.6f, 0.5f, 1.0f) );
	
	// bind uniforms...
	gl::ScopedGlslProg glslProg( mGlsl );
	mGlsl->uniform("Projection", mMayaCam.getCamera().getProjectionMatrix());
	mGlsl->uniform("Modelview", mMayaCam.getCamera().getViewMatrix());
	mGlsl->uniform("NormalMatrix", mNormalMatrix);
	mGlsl->uniform("LightPosition", Vec3f(0.25, 0.25, 1));
	mGlsl->uniform("AmbientMaterial", Vec3f(0.04f, 0.04f, 0.04f));
	mGlsl->uniform("DiffuseMaterial", Vec3f(0, 0.75, 0.75));
	mGlsl->uniform("TessLevelInner", mTessellationInner );
	mGlsl->uniform("TessLevelOuter", mTessellationOuter );
	
	if( mDrawWireframe ){
		gl::enableWireframe();
		mBatch->draw();
		gl::disableWireframe();
	}
	else {
		gl::enable(GL_DEPTH_TEST);
		gl::enable(GL_CULL_FACE);
		mBatch->draw();
		gl::disable(GL_DEPTH_TEST);
		gl::disable(GL_CULL_FACE);
	}

	mParams->draw();
	
	gl::checkError();
}

CINDER_APP_NATIVE( TessellationSampleApp, RendererGl )
