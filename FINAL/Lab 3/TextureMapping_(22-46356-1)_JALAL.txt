#include <GLUT/glut.h>
#include <CoreFoundation/CoreFoundation.h>
#include <iostream>
#include <fstream>
#include <string>

GLuint textureId;

std::string getFullPath(const char* filename) {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFStringRef fileNameCF = CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
    CFURLRef resourceURL = CFBundleCopyResourceURL(mainBundle, fileNameCF, NULL, NULL);
    
    char path[PATH_MAX];
    if (resourceURL && CFURLGetFileSystemRepresentation(resourceURL, true, (UInt8*)path, PATH_MAX)) {
        CFRelease(fileNameCF);
        CFRelease(resourceURL);
        return std::string(path);
    }
    
    CFRelease(fileNameCF);
    return std::string(filename);
}

unsigned char* loadBMP(const char* filename, int& width, int& height) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Image not found: " << filename << std::endl;
        return nullptr;
    }

    unsigned char header[54];
    file.read(reinterpret_cast<char*>(header), 54); // Read BMP header

    width  = *(int*)&header[18];
    height = *(int*)&header[22];
    int imageSize = width * height * 3;

    unsigned char* data = new unsigned char[imageSize];
    file.read(reinterpret_cast<char*>(data), imageSize);
    file.close();

    for (int i = 0; i < imageSize; i += 3) {
        std::swap(data[i], data[i + 2]);
    }

    return data;
}

void loadTexture(const char* filename) {
    int width, height;
    std::string fullPath = getFullPath(filename);
    unsigned char* imageData = loadBMP(fullPath.c_str(), width, height);
    if (!imageData) return;

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
    delete[] imageData;
}

void init() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    loadTexture("image.bmp"); // File must be added to bundle
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D, textureId);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-0.5f, -0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f( 0.5f, -0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f( 0.5f,  0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-0.5f,  0.5f);
    glEnd();

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Texture Mapping - BMP Quad");

    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
