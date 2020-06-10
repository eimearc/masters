#include "egl.h"

#include <thread>

void EGL::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for compilation on OS X.

    window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", NULL, NULL);
    
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		glfwTerminate();
	}
}

void EGL::initGL()
{
    glEnable(GL_DEPTH_TEST);
    createShaders();
    createGrid();
    setupBuffers();
}

void EGL::createGrid()
{
    float gridSize = 2.0f;
    float cubeSize = (gridSize/NUM_CUBES)*0.5;
    grid = Grid(gridSize, cubeSize, NUM_CUBES);
    setupVertices();
}

void EGL::setupVertices()
{
    int i=0;
    const size_t numVerts = 8;
    Vertex vertex = {{}, {1,0,0}};
    for (auto cube : grid.cubes)
    {
        std::vector<glm::vec3> verts = cube.vertices;
        std::vector<uint32_t> ind = cube.indices;
        for(size_t j = 0; j<verts.size(); ++j)
        {
            vertex.pos=verts[j];
            vertex.color=cube.color;
            vertices.push_back(vertex);
        }
        for(size_t j = 0; j<ind.size(); ++j)
        {
            indices.push_back(ind[j]+i*numVerts);
        }
        ++i;
    }
}

void EGL::mainLoop()
{
    int frameNum=0;
    bool timed=false;
    if (FLAGS_num_frames > 0) timed=true;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        bench.numVertices(vertices.size());
        bench.numThreads(FLAGS_num_threads);
        bench.numCubes(FLAGS_num_cubes);
        auto fStartTime = bench.start();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        auto vStartTime = bench.start();
        updateVertexBuffer();
        bench.updateVBOTime(vStartTime);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        bench.frameTime(fStartTime);
        bench.record();

        frameNum++;
        if (timed && frameNum>=FLAGS_num_frames) break;
    }
}

void EGL::cleanup()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
}

void EGL::createShaders()
{
    vertShaderCode = readFile("shaders/shaderGL.vert");
    vertShaderCode.push_back('\0'); // Must be 0 terminated (C-string style).
    fragShaderCode = readFile("shaders/shader.frag");
    fragShaderCode.push_back('\0'); // Must be 0 terminated (C-string style).
    const char* vertexShaderSource = vertShaderCode.data();
    const char* fragmentShaderSource = fragShaderCode.data();

    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void EGL::setupBuffers()
{
    // Set up MVP UBO.
    GLuint bindingPoint = 0;
    ubo = getUBO(WIDTH, HEIGHT);
    ubo.proj[1][1] *= -1; // Y is flipped.
    GLuint blockIndex = glGetUniformBlockIndex(shaderProgram, "ubo");
    glUniformBlockBinding(shaderProgram, blockIndex, bindingPoint);
    glGenBuffers(1, &uboBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, uboBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ubo), &ubo, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uboBuffer);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 1 * sizeof(Vertex), (void*)offsetof(Vertex,pos));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 1 * sizeof(Vertex), (void*)(offsetof(Vertex,color)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0); 
}

void EGL::updateVertexBuffer()
{
    static int counter = 0;

    ubo.model=glm::rotate(glm::mat4(1.0f), 0.01f * glm::radians(90.0f)*counter, glm::vec3(1.0f,0.0f,0.0f));
    glBindBuffer(GL_UNIFORM_BUFFER, uboBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ubo), &ubo, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 1 * sizeof(Vertex), (void*)offsetof(Vertex,pos));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 1 * sizeof(Vertex), (void*)(offsetof(Vertex,color)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0); 

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        // Do not unbind the EBO while a VAO is active as the bound element buffer object is stored in the VAO.
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    counter++;
}