#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Batch.h"
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

class PipelineSampleApp : public AppNative {
public:
//	PipelineSampleApp();
	~PipelineSampleApp();
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
	void loadBufferData();
	void createIcosahedron();
	
	GLuint compileShader( const GLenum type, const string& path );
	void linkProgram( const GLuint program );
	void validateProgram( const GLuint program );
	
	gl::VertBatchRef	mBatch;
	gl::GlslProgRef		mGlsl;
	
	Vec2f mMousePos;
    
    GLuint shaderProgram;
    GLuint vertexArrayObject;
    GLuint vertexBuffer;
	
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

PipelineSampleApp::~PipelineSampleApp()
{
	glDeleteProgram(shaderProgram);
    //GetError();
    glDeleteBuffers(1, &vertexBuffer);
    //GetError();
}

void PipelineSampleApp::setup()
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
	
	mTessellationInner = mTessellationOuter = 3;
	
	this->loadShader();
	this->createIcosahedron();
	
	CameraPersp cam;
	cam.setEyePoint( Vec3f(5.0f, 10.0f, 10.0f) );
	cam.setCenterOfInterestPoint( Vec3f(0.0f, 2.5f, 0.0f) );
	cam.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 1000.0f );
	mMayaCam.setCurrentCam( cam );
}

void PipelineSampleApp::mouseMove( MouseEvent event )
{
	// keep track of the mouse
	mMousePos = event.getPos();
}

void PipelineSampleApp::mouseDown( MouseEvent event )
{
	// let the camera handle the interaction
	mMayaCam.mouseDown( event.getPos() );
}

void PipelineSampleApp::mouseDrag( MouseEvent event )
{
	// keep track of the mouse
	mMousePos = event.getPos();
	
	// let the camera handle the interaction
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void PipelineSampleApp::resize()
{
	// adjust aspect ratio
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
}

void PipelineSampleApp::keyDown( KeyEvent event )
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

GLuint PipelineSampleApp::compileShader(const GLenum type, const string& path)
{
	GLuint shader;
    // load file...
	std::string shader_source = loadString( loadAsset( path ) );
	const GLchar* source = (GLchar*) shader_source.c_str();
    
    shader = glCreateShader(type);
    //GetError();
    glShaderSource(shader, 1, &source, NULL);
    //GetError();
    glCompileShader(shader);
    //GetError();
    
#if defined(DEBUG)
	GLint logLength;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	//GetError();
	if (logLength > 0)
	{
		GLchar *log = (GLchar*)malloc((size_t)logLength);
		glGetShaderInfoLog(shader, logLength, &logLength, log);
		//GetError();
		console() << "Shader compilation failed with error:" << log << std::endl;
		free(log);
		shutdown();
	}
#endif

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	//GetError();
	if (0 == status)
	{
		glDeleteShader(shader);
		console() << "Shader compilation failed!" << std::endl;
		shutdown();
	}

	return shader;
}

void PipelineSampleApp::loadShader()
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
	
//
    if (0 != vertexShader && 0 != fragmentShader && 0 != geometryShader && 0 != tessCtrlShader && 0 != tessEvalShader)
    {
		shaderProgram = glCreateProgram();
		//GetError();

		glAttachShader(shaderProgram, vertexShader  );
		//GetError();
		glAttachShader(shaderProgram, fragmentShader);
		//GetError();
		glAttachShader(shaderProgram, geometryShader);
		//GetError();
		glAttachShader(shaderProgram, tessCtrlShader);
		//GetError();
		glAttachShader(shaderProgram, tessEvalShader);
		//GetError();


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
		//GetError();

		glDeleteShader(vertexShader);
		//GetError();
		glDeleteShader(fragmentShader);
		//GetError();
		glDeleteShader(geometryShader);
		//GetError();
		glDeleteShader(tessCtrlShader);
		//GetError();
		glDeleteShader(tessEvalShader);
		//GetError();
    }
    else {
		//[NSException raise:kFailedToInitialiseGLException format:@"Shader compilation failed."];
		console() << "Shader compilation failed." << std::endl;
		shutdown();
    }
}

void PipelineSampleApp::linkProgram( const GLuint program )
{
	glLinkProgram(program);
	//GetError();

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
    //GetError();
    if (0 == status)
    {
        //[NSException raise:kFailedToInitialiseGLException format:@"Failed to link shader program"];
		console() << "Failed to link shader program" << std::endl;
		shutdown();
    }
}

void PipelineSampleApp::validateProgram( const GLuint program )
{
	GLint logLength;

	glValidateProgram(program);
	//GetError();
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	//GetError();
	if (logLength > 0)
	{
		GLchar* log = (GLchar*) malloc((size_t)logLength);
		glGetProgramInfoLog(program, logLength, &logLength, log);
		//GetError();
		//NSLog(@"Program validation produced errors:\n%s", log);
		console() << "Program validation produced errors:" << log << std::endl;
		free(log);
		shutdown();
	}

	GLint status;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	//GetError();
	if (0 == status)
	{
		//[NSException raise:kFailedToInitialiseGLException format:@"Failed to link shader program"];
		console() << "Failed to link shader program" << std::endl;
		shutdown();
	}
}

void PipelineSampleApp::loadBufferData()
{
	float size = 1.0f;
	Vertex vertexData[6] = {
        { .position = { .x=-size, .y=-size, .z=0.0, .w=1.0 } },
        { .position = { .x=-size, .y= size, .z=0.0, .w=1.0 } },
        { .position = { .x= size, .y= size, .z=0.0, .w=1.0 } },
        { .position = { .x=-size, .y=-size, .z=0.0, .w=1.0 } },
        { .position = { .x= size, .y= size, .z=0.0, .w=1.0 } },
        { .position = { .x= size, .y=-size, .z=0.0, .w=1.0 } }
    };
    
    glGenVertexArrays(1, &vertexArrayObject);
    //GetError();
    glBindVertexArray(vertexArrayObject);
    //GetError();
    
    glGenBuffers(1, &vertexBuffer);
    //GetError();
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    //GetError();
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), vertexData, GL_STATIC_DRAW);
    //GetError();
    
	glEnableVertexAttribArray((GLuint)mPositionIndex);
	//GetError();
	glVertexAttribPointer((GLuint)mPositionIndex, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)offsetof(Vertex, position));
	//GetError();
}

void PipelineSampleApp::createIcosahedron()
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
	
	// Create the VAO:
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	//GetError();
	
	// Create the VBO for positions:
	GLsizei stride = 3 * sizeof(float);
	glGenBuffers(1, &mVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(mPositionIndex);
	glVertexAttribPointer(mPositionIndex, 3, GL_FLOAT, GL_FALSE, stride, 0);
	//GetError();
	
	// Create the VBO for indices:
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(faces), faces, GL_STATIC_DRAW);
	//GetError();
}

void PipelineSampleApp::update()
{
	mNormalMatrix = mMayaCam.getCamera().getViewMatrix().subMatrix33(0,0);
	mNormalMatrix.transpose();
}

void PipelineSampleApp::draw()
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

CINDER_APP_NATIVE( PipelineSampleApp, RendererGl )
