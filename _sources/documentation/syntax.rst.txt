Syntax
======

For the basic syntax see the `ESP-Docs <https://docs.espressif.com/projects/esp-docs/en/latest/writing-documentation/basic-syntax.html>`__ documentation.

I will list a few important ones.

The handling of sections
------------------------

Use ``=`` for sections, ``-`` for subsections and ``"`` for subsubsections.
Here, for example the *"Syntax"* and *"The handling of sections"* titles were created like so:
::

    Syntax
    =============

    The handling of sections
    ------------------------

Here the underscore has to be at least as long as the text itself. Notice, that these sections also appear in the sidebar.

.. note::
    You can also use ``#`` ``*`` and ``^`` if you need more levels in titles. Also note, that it's actually their order that matters, not the character itself when deciding which one is a section, a subsection or a subsubsection, so be consistent.
    Also, to create notes use: ``.. note::``.

Text formatting
---------------

Use 
::
    
    *text* 
    
for *Italic*, 
::
    
    **text** 
    
for **bold**, and 
::
    
    ``text``

for ``literals``.

Code blocks
-----------

The simplest way to start a code block is to write ``::``.

::

    ::

        AT+GMR

This would generate the following:

::

    AT+GMR 

You can also specify the language of the code for better formatting.

::

    .. code-block:: python

        for i in range(10):
        print(i)

Would result in this:

.. code-block:: python

        for i in range(10):
        print(i)

Figures
-------

The output of the following example can be seen in the **Setting up the environment** section.

::

    .. figure:: ../../_static/Region.png
    :align: center
    :scale: 90%
    :alt: Setting Unicode UTF-8

    Setting Unicode UTF-8


Links
-----

There is a lot you can do with `Links <https://docs.espressif.com/projects/esp-docs/en/latest/writing-documentation/links.html>`__.

For our use-case the most important ones are linking to URL-s, other sections of the document, other documents and ESP TRM-s and Datasheets.

Linking to URL-s
""""""""""""""""

::
    
    Welcome to `Espressif <https://www.espressif.com/>`_!

Would render into this:

Welcome to `Espressif <https://www.espressif.com/>`_!

Linking to other sections of the document
"""""""""""""""""""""""""""""""""""""""""

You just have to write the name of the section between backward apostrophes.

::

    `Code blocks`_

`Code blocks`_

Linking to other documents
""""""""""""""""""""""""""

You can link with relative path to the current document, or with absolute path with the root folder being the ``docs`` folder. The latter is recommended.

::

    :doc:`/documentation/index`

:doc:`/documentation/index`

The link text can also be customized:

::

    :doc:`Another Documentation </documentation/index>`

:doc:`Another Documentation </documentation/index>`


Linking to a Specific Place of Other Documents in the Same Project
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

You have to place an "anchor" in the document you want to link to, and then reference that in your current document.

For example, if you place the following anchor into *"structure.rst"*, which is the reStructuredText file for the Structure section like this:

::

    .. _example-structure-1:

And then reference it like this:

::

    :ref:`_example-structure-1`

Then you can jump to that anchor.

:ref:`Building the documentation <example-structure-1>`


To-Do Notes
-----------

Syntax and example:

::

    .. todo::
        Add a package diagram.

If you add ``.. todolist::`` to a reST file, the directive will be replaced by a list of all to-do notes from the whole documentation.

.. note::
    You dont have to do this, I've already created a TODO list directory. Just throughout the documentation wherever you want to, add ``.. todo::``-s and they will accumulate there, with information about their locations too.
    They will also appear in the same place where you put the ``.. todo::``.
