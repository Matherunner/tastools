.. TasTools documentation master file, created by
   sphinx-quickstart on Mon May 12 17:03:30 2014.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Half-Life Physics
=================

.. caution::
   This documentation is incomplete and may contain outdated information.  Due
   to real life commitments I am not able to update this documentation as
   quickly as I would like to.  Nevertheless, this is still the best place to
   go if you seek to deepen your theoretical knowledge of Half-Life and gain a
   further appreciation for Half-Life TASes.

This is the documentation for TasTools mod and some aspects of the physics of
the latest Half-Life version that may be important for TAS runners to know
about.  This is currently the most detailed and up-to-date documentation about
Half-Life player physics available.  The latest build number at the time of
writing is 6153.  Some of the information here are Linux-specific, but most
descriptions about Half-Life are applicable to the Windows and Mac versions as
well.  You might need to familiarise yourself with the Half-Life SDK codebase
and have a mature mathematical skill to be able to follow this document.  The
official SDK code can be found here__.

__ https://github.com/ValveSoftware/halflife

We will use the angle bracket :math:`\langle \rangle` to write out the
components of vectors explicitly.  Vectors are otherwise denoted in boldface.
For example, :math:`\mathbf{v} = \langle 3, 4, 5 \rangle` means the vector
:math:`\mathbf{v}` has :math:`x`, :math:`y` and :math:`z` components 3, 4 and 5
respectively.  If a hat is written on top of the vector, then it must be a unit
vector.  For example, :math:`\mathbf{\hat{v}} = \mathbf{v} /
\lVert\mathbf{v}\rVert`.

All angles are assumed to be in radians unless stated otherwise.  The
:math:`{}^\circ` symbol will be used to denote angles in degrees.

**Contents**:

.. toctree::
   :maxdepth: 2

   basicphy
   strafing
   algorithms
   health
   tastools
