Fundamentals
============

Notation
--------

One of the most important mathematical objects in discussions of Half-Life physics is the Euclidean vector. All vectors are in either :math:`\mathbb{R}^2` or :math:`\mathbb{R}^3`, where :math:`\mathbb{R}` denotes the real numbers. This is sometimes not specified explicitly if the contextual clues are sufficient for disambiguation.

All vectors are written in boldface like so:

.. math:: \mathbf{v}

Every vector has an associated length, which is referred to as the *norm*. The norm of some vector :math:`\mathbf{v}` is thus denoted as

.. math:: \lVert\mathbf{v}\rVert

A vector of length one is called a *unit vector*. So the unit vector in the direction of some vector :math:`\mathbf{v}` is written with a hat:

.. math:: \mathbf{\hat{v}} = \frac{\mathbf{v}}{\lVert\mathbf{v}\rVert}

There are three special unit vectors, namely


.. math:: \mathbf{\hat{i}} \quad \mathbf{\hat{j}} \quad \mathbf{\hat{k}}

These vectors point towards the positive :math:`x`, :math:`y` and :math:`z` axes respectively.

Every vector also has components in each axis. For a vector in :math:`\mathbb{R}^2`, it has an :math:`x` component and a :math:`y` component. A vector in :math:`\mathbb{R}^3` has an additional :math:`z` component. To write out the components of a vector explicitly, we have

.. math:: \mathbf{v} = \langle v_x, v_y, v_z\rangle

This is equivalent to writing :math:`\mathbf{v} = v_x \mathbf{\hat{i}} + v_y \mathbf{\hat{j}} + v_z \mathbf{\hat{k}}`. However, we never write out the components this way in this documentation as it is tedious. Notice that we are writing vectors as row vectors. This will be important to keep in mind when we apply matrix transformations to vectors.

The dot product between two vectors :math:`\mathbf{a}` and :math:`\mathbf{b}` is written as

.. math:: \mathbf{a} \cdot \mathbf{b}

On the other hand, the cross product between :math:`\mathbf{a}` and :math:`\mathbf{b}` is

.. math:: \mathbf{a} \times \mathbf{b}

Viewangles
----------

The term *viewangles* is usually associated with the player entity. The viewangles refer to a group of three angles which describe the player's view orientation. We call these angles *yaw*, *pitch* and *roll*. Mathematically, we denote the yaw by

.. math:: \vartheta

and the pitch by

.. math:: \varphi

Note that these are different from :math:`\theta` and :math:`\phi`. We do not have a mathematical symbol for roll as it is rarely used. In mathematical discussions, the viewangles are assumed to be in *radians* unless stated otherwise. However, do keep in mind that they are stored in degrees in the game.

One way to change the yaw and pitch is by moving the mouse. This is not useful for tool-assisted speedrunning, however. A better method for precise control of the yaw and pitch angles is by issuing the commands ``+left``, ``+right``, ``+up`` ,or ``+down``. When these commands are active, the game increments or decrements the yaw or pitch by a certain controllable amount per frame. The amounts can be controlled by adjusting the variables ``cl_yawspeed`` and ``cl_pitchspeed``. For instance, when ``+right`` is active, the game multiplies the value of ``cl_yawspeed`` by the frame time, then subtracts the result from the yaw angle.

View vectors
------------

There are two vectors associated with the player's viewangles. These are called the *view vectors*. For discussions in 3D space, they are defined to be

.. math::
	\begin{align*}
	\mathbf{\hat{f}} &:= \langle \cos\vartheta \cos\varphi, \sin\vartheta \cos\varphi, -\sin\varphi \rangle \\
	\mathbf{\hat{s}} &:= \langle \sin\vartheta, -\cos\vartheta, 0 \rangle
	\end{align*}

We will refer to the former as the *unit forward vector* and the latter as the *unit right vector*. The negative sign for :math:`f_z` is an idiosyncrasy of the GoldSrc engine inherited from Quake. This is the consequence of the fact that looking up gives negative pitch and vice versa.

We sometimes restrict our discussions to the horizontal plane, such as in the description of strafing. In this case we assume :math:`\varphi = 0` and define

.. math::
	\begin{align*}
	\mathbf{\hat{f}} &:= \langle \cos\vartheta, \sin\vartheta \rangle \\
	\mathbf{\hat{s}} &:= \langle \sin\vartheta, -\cos\vartheta \rangle
	\end{align*}

Such restriction is equivalent to projecting the :math:`\mathbf{\hat{f}}` vector onto the :math:`xy` plane, provided the original vector is not vertical.

The above definitions are not valid if the roll is nonzero. Nevertheless, such situations are extremely rare in practice.

.. _entities:

Entities
--------

.. _tracing:

Tracing
-------

Tracing is one of the most important computations done by the game. Tracing is done countless times per frame, and it is vital to how entities interact with one another.