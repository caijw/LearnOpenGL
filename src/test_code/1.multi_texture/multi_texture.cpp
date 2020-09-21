#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <vector>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "   FragColor = texture(texture1, TexCoord);\n"
    "}\n\0";


// settings
unsigned int SCR_WIDTH = 0;
unsigned int SCR_HEIGHT = 0;

// 高度相同，宽度不同的图片，从左向右渲染
std::vector<std::string> imagePaths = {
    "resources/textures/container.jpg",
    "resources/textures/container2.png",
    "resources/textures/container2_specular.png"
};

class Box {
public:
    double x;
    double y;
    double width;
    double height;
};

class Image {
public:
    Image(const std::string &path, int width, int height, int nrChannels, unsigned char *data)
        : path(path), width(width), height(height), nrChannels(nrChannels), data(data) {}
    std::string path = "";
    int width = 0;
    int height = 0;
    int nrChannels = 0;
    unsigned char *data;
    unsigned int texture;
    Box box;
    unsigned int VBO, VAO, EBO;
    float vertices[20];
    unsigned int indices[6];
};

int main()
{
    std::vector<Image> images;

    for (auto &imagePath : imagePaths) {
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        unsigned char *data = stbi_load(FileSystem::getPath(imagePath).c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            std::cout << "container.jpg width: " << width << " height: " << height << " nrChannels: " << nrChannels << std::endl;
            images.emplace_back(imagePath, width, height, nrChannels, data);
            SCR_WIDTH += width;
            SCR_HEIGHT = height;
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
            return -1;
        }
    }

    std::cout << "SCR_WIDTH:" << SCR_WIDTH << " SCR_HEIGHT:" << SCR_HEIGHT << std::endl;
    
    double left = 0;
    for (auto &image : images) {
        image.box.x = 2.0 * left / SCR_WIDTH - 1;
        image.box.y = 1;
        image.box.width = ( 2.0f * image.width) / SCR_WIDTH;
        image.box.height = ( 2.0f * image.height) / SCR_HEIGHT;
        left += image.width;
        std::cout << "image" << std::endl;
        std::cout << "box: (" << image.box.x << "," << image.box.y << "," << image.box.width << "," << image.box.height << ")" << std::endl;
    }

    
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);



    // build and compile our shader zprogram
    // ------------------------------------
    // Shader ourShader("6.1.coordinate_systems.vs", "6.1.coordinate_systems.fs");

    for (auto &image : images) {
        int index = 0;
        // top right
        image.vertices[index++] = image.box.x + image.box.width;
        image.vertices[index++] = image.box.y;
        image.vertices[index++] = 0;
        image.vertices[index++] =  1.0f;
        image.vertices[index++] = 1.0f;
        std::cout << "image" << std::endl;
        std::cout << "top right:(" << image.vertices[index - 5] << "," << image.vertices[index - 4] << "," << image.vertices[index - 3] << "," << image.vertices[index - 2] << "," << image.vertices[index - 1] << ")" << std::endl;
        // bottom right
        image.vertices[index++] = image.box.x + image.box.width;
        image.vertices[index++] = image.box.y - image.box.height;
        image.vertices[index++] = 0;
        image.vertices[index++] =  1.0f;
        image.vertices[index++] = 0.0f;
        std::cout << "bottom right:(" << image.vertices[index - 5] << "," << image.vertices[index - 4] << "," << image.vertices[index - 3] << "," << image.vertices[index - 2] << "," << image.vertices[index - 1] << ")" << std::endl;
        // bottom left
        image.vertices[index++] = image.box.x;
        image.vertices[index++] = image.box.y - image.box.height;
        image.vertices[index++] = 0;
        image.vertices[index++] =  0.0f;
        image.vertices[index++] = 0.0f;
        std::cout << "bottom left:(" << image.vertices[index - 5] << "," << image.vertices[index - 4] << "," << image.vertices[index - 3] << "," << image.vertices[index - 2] << "," << image.vertices[index - 1] << ")" << std::endl;
        // top left 
        image.vertices[index++] = image.box.x;
        image.vertices[index++] = image.box.y;
        image.vertices[index++] = 0;
        image.vertices[index++] =  0.0f;
        image.vertices[index++] = 1.0f;
        std::cout << "top left:(" << image.vertices[index - 5] << "," << image.vertices[index - 4] << "," << image.vertices[index - 3] << "," << image.vertices[index - 2] << "," << image.vertices[index - 1] << ")" << std::endl;
        index = 0;
        // first triangle
        image.indices[index++] = 0;
        image.indices[index++] = 1;
        image.indices[index++] = 3;
        // second triangle
        image.indices[3] = 1;
        image.indices[4] = 2;
        image.indices[5] = 3;

        glGenVertexArrays(1, &image.VAO);
        glGenBuffers(1, &image.VBO);
        glGenBuffers(1, &image.EBO);

        glBindVertexArray(image.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, image.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(image.vertices), image.vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, image.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(image.indices), image.indices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // texture coord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

    }

    for (auto &image : images) {
        glGenTextures(1, &image.texture);
        glBindTexture(GL_TEXTURE_2D, image.texture);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load image, create texture and generate mipmaps
        if (image.nrChannels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width, image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);
        } else if (image.nrChannels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(image.data);
        image.data = nullptr;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    // ourShader.use();
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0); 
    // ourShader.setInt("texture1", 0);
    // ourShader.setInt("texture2", 1);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // activate shader
        // ourShader.use();
        glUseProgram(shaderProgram);

        for (auto &image : images) {
        // bind textures on corresponding texture units
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, image.texture);
            // create transformations
            glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            // model = glm::translate(model, glm::vec3( 1.0f,  1.0f, 0.0f));
            // retrieve the matrix uniform locations
            unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


            // render container
            glBindVertexArray(image.VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (auto &image : images) {
        // optional: de-allocate all resources once they've outlived their purpose:
        // ------------------------------------------------------------------------
        glDeleteVertexArrays(1, &image.VAO);
        glDeleteBuffers(1, &image.VBO);
        glDeleteBuffers(1, &image.EBO);
    }



    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}