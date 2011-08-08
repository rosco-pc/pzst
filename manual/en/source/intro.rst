.. vim: textwidth=80 :
Introduction
============

Propeller Zone Spin Tool (PZST in short) is a development environment for
Propeller microcontroller from `Parallax, Inc.
<http://www.parallax.com>`_.

PZST includes code editor and loader. For compilation, `bstc <http://www.fnarfbargle.com/bst.html>`_
optimizing compiler is used.

PZST's functionality is similar to  
`Propeller/Spin Tool <http://www.parallax.com/tabid/832/Default.aspx#Software>`_ and
`bst <http://www.fnarfbargle.com/bst.html>`_, with few improvements: 
    * unlike Propeller/Spin Tool and bst, PZST supports text strings in
      national encodings. Propeller/Spin Tool and bst only work with Latin-1 encoding
    * PZST  correctly works with USB-to-serial converters based on CP210x chips.
      bst turned out to work unreliably with CP210x under certain Linux kernels
    * additional code editor features, such as code completion, call tips and
      quick search
      
Currently, PZST works in Microsoft Windows (Windows XP and newer) and Linux.
Porting to Mac OS X is in progress.


