#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Utilities.h"
#include "cinder/Matrix33.h"
#include "cinder/Matrix44.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Vector.h"

using namespace ci;
using namespace ci::app;
using namespace std;

typedef struct
{
    float x,y;
} Vector2;

typedef struct
{
    float x,y,z,w;
} Vector4;

typedef struct
{
    float r,g,b,a;
} Colour;

typedef struct
{
    Vector4 position;
} Vertex;

class TessellationSampleApp : public AppNative {
public:
//	TessellationSampleApp();
	~TessellationSampleApp();
	void setup();
	void mouseMove( MouseEvent event );
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	
	void resize();
	
	void keyDown( KeyEvent event );
	void update();
	void draw();
	
private:
	void loadShader();
	void createIcosahedron();
	
	GLuint compileShader( const GLenum type, const string& path );
	void linkProgram( const GLuint program );
//	void validateProgram( const GLuint program );
	
	gl::VertBatchRef	mBatch;
	gl::GlslProgRef		mGlsl;
	
	Vec2f mMousePos;
    
    GLuint shaderProgram;
	gl::VaoRef vertexArrayObject;
	gl::VboRef vertexBufferObject;
	gl::VboRef indexBufferObject;
	
	GLuint mVertBuffer;
	GLuint mIndexBuffer;
	GLuint mPositionIndex;
	GLuint mTessellationInner;
	GLuint mTessellationOuter;
	
	GLint projectionMatLoc;
	GLint modelViewMatLoc;
	GLint normalMatLoc;
	GLint lightPosLoc;
	GLint ambientColorLoc;
	GLint diffuseColorLoc;
	GLint tessellationInnerLoc;
	GLint tessellationOuterLoc;
	
	GLsizei mIndexCount;
	
	MayaCamUI mMayaCam;
	Matrix33f mNormalMatrix;
};

TessellationSampleApp::~TessellationSampleApp()
{
	glDeleteProgram(shaderProgram);
    
    //glDeleteBuffers(1, &vertexBuffer);
    
}

void TessellationSampleApp::setup()
{
	// setup shader
//	try {
//		mGlsl = gl::GlslProg::create( gl::GlslProg::Format().vertex( loadAsset( "basic.vert" ) )
//									 .fragment( loadAsset( "basic.frag" ) )
//									 .geometry( loadAsset( "basic.geom" ) ) );
//	}
//	catch( gl::GlslProgCompileExc ex ) {
//		cout << ex.what() << endl;
//		shutdown();
//	}
	
	// setup VertBatch with a single point at the origin
//	mBatch = gl::VertBatch::create();
//	mBatch->vertex( Vec2f::zero() );
//	mBatch->color( 1.0f, 0.0f, 0.0f );
	
	vertexArrayObject = gl::Vao::create();
	vertexBufferObject = gl::Vbo::create(GL_ARRAY_BUFFER);
	indexBufferObject = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER);
	
	mTessellationInner = mTessellationOuter = 3;
	
	this->loadShader();
	this->createIcosahedron();
	
	CameraPersp cam;
	cam.setEyePoint( Vec3f(5.0f, 10.0f, 10.0f) );
	cam.setCenterOfInterestPoint( Vec3f(0.0f, 2.5f, 0.0f) );
	cam.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 1000.0f );
	mMayaCam.setCurrentCam( cam );
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
	else if( event.getCode() == KeyEvent::KEY_LEFT ){
		mTessellationOuter++;
	}
	else if( event.getCode() == KeyEvent::KEY_RIGHT ){
		if (mTessellationOuter > 1)
			mTessellationOuter--;
	}
	else if( event.getCode() == KeyEvent::KEY_f ){
		setFullScreen( !isFullScreen() );
	}
}

GLuint TessellationSampleApp::compileShader(const GLenum type, const string& path)
{
	GLuint shader;
    // load file...
	std::string shader_source = loadString( loadAsset( path ) );
	const GLchar* source = (GLchar*) shader_source.c_str();
    
    shader = glCreateShader(type);
    
    glShaderSource(shader, 1, &source, NULL);
    
    glCompileShader(shader);
    
    
#if defined(DEBUG)
	GLint logLength;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	
	if (logLength > 0)
	{
		GLchar *log = (GLchar*)malloc((size_t)logLength);
		glGetShaderInfoLog(shader, logLength, &logLength, log);
		
		console() << "Shader compilation failed with error:" << log << std::endl;
		free(log);
		shutdown();
	}
#endif

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (0 == status)
	{
		glDeleteShader(shader);
		console() << "Shader compilation failed!" << std::endl;
		shutdown();
	}

	return shader;
}

void TessellationSampleApp::loadShader()
{
	GLuint vertexShader;
    GLuint fragmentShader;
    GLuint geometryShader;
    GLuint tessCtrlShader;
    GLuint tessEvalShader;
    
	vertexShader = this->compileShader(GL_VERTEX_SHADER, "shaders/pipeline_vert.glsl");
	fragmentShader = this->compileShader(GL_FRAGMENT_SHADER, "shaders/pipeline_frag.glsl");
	geometryShader = this->compileShader(GL_GEOMETRY_SHADER, "shaders/pipeline_geo.glsl");
	tessCtrlShader = this->compileShader(GL_TESS_CONTROL_SHADER, "shaders/pipeline_tess_c.glsl");
	tessEvalShader = this->compileShader(GL_TESS_EVALUATION_SHADER, "shaders/pipeline_tess_e.glsl");
	
    if (0 != vertexShader && 0 != fragmentShader && 0 != geometryShader && 0 != tessCtrlShader && 0 != tessEvalShader)
    {
		shaderProgram = glCreateProgram();

		glAttachShader(shaderProgram, vertexShader  );
		glAttachShader(shaderProgram, fragmentShader);
		glAttachShader(shaderProgram, geometryShader);
		glAttachShader(shaderProgram, tessCtrlShader);
		glAttachShader(shaderProgram, tessEvalShader);
		
		glBindFragDataLocation(shaderProgram, 0, "FragColor");

		this->linkProgram(shaderProgram);
		
		// get shader uniforms
		projectionMatLoc = glGetUniformLocation( shaderProgram, "Projection" );
		modelViewMatLoc = glGetUniformLocation( shaderProgram, "Modelview" );
		normalMatLoc = glGetUniformLocation( shaderProgram, "NormalMatrix" );
		lightPosLoc = glGetUniformLocation( shaderProgram, "LightPosition" );
		ambientColorLoc = glGetUniformLocation( shaderProgram, "AmbientMaterial" );
		diffuseColorLoc = glGetUniformLocation( shaderProgram, "DiffuseMaterial" );
		tessellationInnerLoc = glGetUniformLocation( shaderProgram, "TessLevelInner" );
		tessellationOuterLoc = glGetUniformLocation( shaderProgram, "TessLevelOuter" );

		// get vert attributes
		mPositionIndex = glGetAttribLocation( shaderProgram, "Position" );
		

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteShader(geometryShader);
		glDeleteShader(tessCtrlShader);
		glDeleteShader(tessEvalShader);
		
    }
    else {
		//[NSException raise:kFailedToInitialiseGLException format:@"Shader compilation failed."];
		console() << "Shader compilation failed." << std::endl;
		shutdown();
    }
}

void TessellationSampleApp::linkProgram( const GLuint program )
{
	glLinkProgram(program);
	

#if defined(DEBUG)
//	GLint logLength;
//
//	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
//	GetError();
//	if (logLength > 0)
//	{
//		GLchar *log = malloc((size_t)logLength);
//		glGetProgramInfoLog(program, logLength, &logLength, log);
//		GetError();
//		NSLog(@"Shader program linking failed with error:\n%s", log);
//		free(log);
//	}
#endif
    
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    
    if (0 == status)
    {
        //[NSException raise:kFailedToInitialiseGLException format:@"Failed to link shader program"];
		console() << "Failed to link shader program" << std::endl;
		shutdown();
    }
}

/*
void TessellationSampleApp::validateProgram( const GLuint program )
{
	GLint logLength;

	glValidateProgram(program);
	
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	
	if (logLength > 0)
	{
		GLchar* log = (GLchar*) malloc((size_t)logLength);
		glGetProgramInfoLog(program, logLength, &logLength, log);
		
		//NSLog(@"Program validation produced errors:\n%s", log);
		console() << "Program validation produced errors:" << log << std::endl;
		free(log);
		shutdown();
	}

	GLint status;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	
	if (0 == status)
	{
		//[NSException raise:kFailedToInitialiseGLException format:@"Failed to link shader program"];
		console() << "Failed to link shader program" << std::endl;
		shutdown();
	}
}
*/

void TessellationSampleApp::createIcosahedron()
{
	const int faces[] = {
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
	
	mIndexCount = sizeof(faces) / sizeof(faces[0]);
	
	vertexArrayObject->bind();
	
	// Create the VBO for positions:
	GLsizei stride = 3 * sizeof(float);
	vertexBufferObject->bind();
	vertexBufferObject->bufferData(sizeof(verts), verts, GL_STATIC_DRAW);
	gl::enableVertexAttribArray( (GLuint)mPositionIndex );
	gl::vertexAttribPointer( (GLuint)mPositionIndex, 3, GL_FLOAT, GL_FALSE, stride, 0 );
	
	// Create the VBO for indices:
	indexBufferObject->bind();
	indexBufferObject->bufferData( sizeof(faces), faces, GL_STATIC_DRAW );
}

void TessellationSampleApp::update()
{
	mNormalMatrix = mMayaCam.getCamera().getViewMatrix().subMatrix33(0,0);
	mNormalMatrix.transpose();
}

void TessellationSampleApp::draw()
{
	// clear out the window with black
//	gl::clear( Color( 0, 0, 0 ) );
	
//	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
//	gl::translate( getWindowCenter() );
	
//	gl::ScopedGlslProg glslProg( mGlsl );
//	mGlsl->uniform( "uNumSides", mNumSides );
//	mGlsl->uniform( "uRadius", mRadius );
//	mBatch->draw();
	
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glUseProgram(shaderProgram);
	
	glUniform1f( tessellationInnerLoc, mTessellationInner );
	glUniform1f( tessellationOuterLoc, mTessellationOuter );
	
	glUniform3f(ambientColorLoc, 0.04f, 0.04f, 0.04f);
	glUniform3f(diffuseColorLoc, 0, 0.75, 0.75);
	glUniform3f(lightPosLoc, 0.25, 0.25, 1);
	
	//gl::setMatrices( mMayaCam.getCamera() );
	glUniformMatrix4fv(projectionMatLoc, 1, GL_FALSE, mMayaCam.getCamera().getProjectionMatrix().m);
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, mMayaCam.getCamera().getViewMatrix().m);
	glUniformMatrix4fv(normalMatLoc, 1, GL_FALSE, mNormalMatrix.m);
	
	glClearColor(0.7f, 0.6f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glPatchParameteri(GL_PATCH_VERTICES, 3);	// triggers a crash for some reason...
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	//glDrawArrays(GL_PATCHES, 0, 4);
	glDrawElements(GL_PATCHES, mIndexCount, GL_UNSIGNED_INT, 0);
	
    glUseProgram( 0 );
}

CINDER_APP_NATIVE( TessellationSampleApp, RendererGl )
