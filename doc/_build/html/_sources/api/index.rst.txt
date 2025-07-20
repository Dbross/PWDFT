API Reference
=============

.. toctree::
   :maxdepth: 2

   keywords

Overview
--------

This section provides comprehensive documentation of all PWDFT input keywords and parameters.

**Keyword Categories:**

* **Control Parameters** - Calculation type, convergence, and output options
* **System Definition** - Atomic structure, cell parameters, and pseudopotentials
* **Electronic Structure** - SCF parameters, mixing, and smearing
* **Advanced Features** - DFT+U, spin-orbit coupling, and more
* **Numerical Stability** - NaN detection and fallback recovery mechanisms

**Keyword Format:**

Each keyword entry includes:

* **Description**: What the parameter controls
* **Data Type**: Integer, float, string, logical
* **Default Value**: Standard setting if not specified
* **Valid Range**: Acceptable values and constraints
* **Physical Meaning**: Connection to underlying physics
* **Examples**: Sample input files and usage

**Complete Keyword Reference:**

* :doc:`keywords` - Comprehensive list of all input keywords with detailed descriptions

**Search and Navigation:**

* **Full-text search** across all documentation
* **Cross-references** between related keywords
* **Index** of all functions and keywords

**Contributing:**

To improve the API documentation:

1. **Add detailed comments** to source code functions
2. **Use consistent formatting** for parameter descriptions
3. **Include examples** in comments where helpful
4. **Update this documentation** when adding new features

**Building the Documentation:**

.. code-block:: bash

   # Build Sphinx documentation
   cd doc
   make html
   make latexpdf

The generated documentation will be available in `doc/_build/html/` and `doc/_build/latex/`. 