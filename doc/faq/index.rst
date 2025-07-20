Frequently Asked Questions
=========================

This section addresses common questions and issues encountered when using PWDFT.

Introduction
-----------

This FAQ section provides answers to common questions about PWDFT usage, convergence issues, performance optimization, and troubleshooting.

**Common Topics**:
* SCF convergence problems
* Performance optimization
* System classification issues
* Input file syntax
* Error messages and solutions
* NaN detection and numerical stability

**Getting Help**:
If you don't find the answer to your question here, please:
1. Check the :doc:`../theory/index` section for detailed explanations
2. Review the :doc:`../tutorials/index` for examples
3. Report issues on the GitHub repository
4. Contact the development team

**Common Issues and Solutions**:

**Q: My calculation is producing NaN values. What should I do?**
A: PWDFT includes automatic NaN detection and fallback recovery. Check the output for "NaN/Inf detected" messages. The system will automatically attempt recovery by reinitializing wavefunctions and using stabilization techniques.

**Q: SCF is not converging. How can I improve convergence?**
A: Try adjusting the SCF mixing parameters, use different initial wavefunction guesses, or enable adaptive mixing with `scf_adaptive_mixing`. See the :doc:`../tutorials/best_practices` for detailed guidance.

**Q: How do I choose the right cutoff energy?**
A: Start with 60-80 Rydberg for most systems. For metals, you may need higher values. Use the convergence guide in :doc:`../tutorials/best_practices` to systematically test different values.

**Q: What's the difference between different SCF algorithms?**
A: Different algorithms have different convergence characteristics. Grassmann conjugate gradient is often most robust, while Anderson mixing can be faster for well-behaved systems. See the theory documentation for details.

For more detailed information, see the main documentation sections and API reference. 