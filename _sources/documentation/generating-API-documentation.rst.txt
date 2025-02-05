Generating API documentation
============================

API documentation can be automatically generated from special comments in the header files, and included in reST files.
You can find the syntax for writing API documentation `here <https://docs.espressif.com/projects/esp-docs/en/latest/writing-documentation/writing-api-documentation.html>`__.
There is indepth description about writing documentation for functions and also about in-body comments (for documenting members of a structure, enum, or a class for exampe).
I will showcase the most important sytnax elements.

.. note::
    How the examples shown here will render can be seen at the end of this page, under `API Reference`_.

Functions, classes
------------------

When documenting a function or a class, start with a special comment block before it.

::

    /**
    * 
    *
    */

Inside this block, there are a number of macros that you can use to describe the code.

- ``@brief`` signals a brief description of the function.
- ``@note`` serves the same purpose as ``.. note::``.
- ``@return`` to specify the return values and their meaning.
- ``@param[in] and @param[out]`` for describing parameters.

.. note::
    After the brief description add a blank line, and add more description if needed.

Example usage:

::

    /**
    * @brief Starts the camera
    * 
    * A more indepth description could be added here if needed.
    * 
    * @return 
    *     - ESP_OK : camera initialized successfully
    *     - ESP_FAIL : couldn't initialize camera
    *     - ESP_ERR_NOT_SUPPORTED : JPEG format not supported on this sensor
    */
    esp_err_t start(); 

For in-body comments:

::

    #define CAM_PIN_PWDN -1          /*!<GPIO_NUM_38 */
    #define CAM_PIN_RESET -1         /*!<GPIO_NUM_39 */
    #define CAM_PIN_XCLK GPIO_NUM_4  /*!<XC */

.. note::
    When you start documenting a header file, you have to document all of its functions.

When building locally, ``run_doxygen.py`` generates ``.inc`` files from input header files defined in ``Doxyfile`` according to configuration, and places the ouput files in ``_build/$(language)/$(target)/inc`` directory.
If you want to include an API documentation in a reST file, just add the following: ``.. include-build-file:: inc/name-of-the-header.inc``.

For example:

::

    API Reference
    -------------

    .. include-build-file:: inc/camera.inc

API Reference
-------------

.. include-build-file:: inc/camera.inc