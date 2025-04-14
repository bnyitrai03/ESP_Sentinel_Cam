Structure
=========

Using the ESP-Docs tools
------------------------

In order to use the ESP-Docs tools, you have to have the following structure:

::

    docs
    |--en
       |--index.rst
       |--conf.py
    |--conf_common.py
    |--Doxyfile
    |--_static

Where docs is on the same level as your components folder.

.. note::
    If you want to split your documentation into sections, give the outline of the table of contents (**toctree**) int the **index.rst** file. Then add more folders to the **en** folder based on that, each with their necessray .rst files.

In the **Doxyfile** under ``INPUTS`` you have to add the path of the header files from which you want to generate API documentation.

.. note::
    The path must start with ``$(PROJECT_PATH)``.

In ``conf_common.py`` you can add the third party extensions you wish to use. You also have to add these to the ``requirements.txt``.

If you want to add figures, you have to store them in the ``_static`` folder. Then, you can reference them like this (the output can be seen in Documentation/Setting up the environment):


Building the documentation
--------------------------

.. _example-structure-1:

:: 

    cd docs
    build-docs build

If all goes well, this will generate an ``index.html`` file inside ``docs\_build\en\generic\html``.

In case troubleshooting is needed. the error outputs can be found in ``docs\_build\en\generic\doxygen-warning-log-sanitized.txt`` and ``docs\_build\en\generic\sphinx-warning-log-sanitized.txt``.