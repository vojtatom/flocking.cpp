#pragma once
#include <glm/glm.hpp>

/**
 * @brief Camera move direction enum
 */
enum CameraMoveDirection
{
    /**
     * @brief Movement in direction of the X-axis
     */
    X,
    /**
     * @brief Movement in direction of the Y-axis
     */
    Y,
    /**
     * @brief Movement in direction of the Z-axis
     */
    Z
};

/**
 * @brief Class representing the Camera in the scene
 * 
 * Produces the view and perspective matrices.
 */
struct Camera
{
    /**
     * @brief Construct a new Camera object
     * 
     * @param _pos vector representing the position of the camera
     * @param _center vector representing the center of the scene (pivot point for rotation)
     * @param _up camera up vector
     */
    Camera(glm::vec3 _pos, glm::vec3 _center, glm::vec3 _up);

    /**
     * @brief Update the perspective matrix
     * 
     * @param w New width of the viewport
     * @param h New height of the viewport
     */
    void resize(int w, int h);

    /**
     * @brief Move the camera in a direction of view
     * 
     * @param dir Direction of movement
     * @param delta Movement factor specifying how much to move
     */
    void move(CameraMoveDirection dir, float delta);

    /**
     * @brief Set the camera position
     * 
     * @param x x coordinate of the position
     * @param y y coordinate of the position
     * @param z z coordinate of the position
     */
    void set(float x, float y, float z);

    /**
     * @brief Rotate the camera based on the mouse movement
     * 
     * The function adjusts the target position after the rotation,
     * producing smooth transition on the screen
     * 
     * @param deltaX mouse delta in the x axis
     * @param deltaY mouse delta in the y axis
     */
    void rotate(int deltaX, int deltaY);

    /**
     * @brief Rotate the camera based on the mouse movement
     * 
     * The function adjusts the actual position of the camera,
     * producing realtime change of the camera position
     * 
     * @param deltaX mouse delta in the x axis
     * @param deltaY mouse delta in the y axis
     */
    void rotateDirect(float deltaX, float deltaY);

    /**
     * @brief Calculate the position fo the camera in the next frame
     * 
     * Allows for smooth camera position interpolation
     */
    void frame(bool run);

    /**
     * @brief Target camera position
     */
    glm::vec3 pos;

    /**
     * @brief Target camera rotation pivot point
     */
    glm::vec3 center;

    /**
     * @brief Target camera up vector
     */
    glm::vec3 up;

    /**
     * @brief Current camera position
     */
    glm::vec3 sPos;
    /**
     * @brief Current camera center
     */
    glm::vec3 sCenter;
    /**
     * @brief Current camera up vector
     */
    glm::vec3 sUp;

    /**
     * @brief Position momentum
     * 
     * Allows for smooth interpolation between target positions
     */
    float posMomentum;
    /**
     * @brief Rotation momentum
     * 
     * Allows for smooth interpolation between target positions
     */
    float rotMomentum;
    /**
     * @brief Pivot point momentum
     * 
     * Allows for smooth interpolation between target positions
     */
    float centerMomentum;

    /**
     * @brief Current view matrix
     */
    glm::mat4 view;

    /**
     * @brief Current projection matrix
     */
    glm::mat4 projection;

    /**
     * @brief Field of view in radians
     */
    float fovy;

    /**
     * @brief Aspect ratio
     */
    float aspect;

    /**
     * @brief Near plane distance
     */
    float fnear;

    /**
     * @brief Far plane distance
     */
    float ffar;

    size_t width, height;
};
