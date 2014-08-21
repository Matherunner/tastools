Health, damage, other entities
==============================


Health and damage
-----------------

Suppose :math:`\mathcal{H}` and :math:`\mathcal{A}` are the health and armour
respectively, while :math:`\mathcal{H}'` and :math:`\mathcal{A}'` are new
health and armour.  Suppose :math:`D` is the original damage received by the
player.  Assuming the damage type is not ``DMG_FALL`` and not ``DMG_DROWN``,
then

.. math:: \mathcal{H}' = \mathcal{H} - \Delta\mathcal{H}
          \quad\quad\quad
          \mathcal{A}' =
          \begin{cases}
          0 & \text{if } \mathcal{A} = 0 \\
          \max(0, \mathcal{A} - 2D/5) & \text{otherwise}
          \end{cases}

where

.. math:: \Delta\mathcal{H} =
          \begin{cases}
          \operatorname{trunc}(D - 2\mathcal{A}) & \text{if } \mathcal{A}' = 0 \\
          \operatorname{trunc}(D/5) & \text{otherwise}
          \end{cases}

If the damage type if ``DMG_FALL`` or ``DMG_DROWN``, then the armour value will
remain the same, with :math:`\Delta\mathcal{H} = \operatorname{trunc}(D)`.
Sometimes the player velocity will be modified upon receiving damage, forming
the foundation for a variety of damage boosts.  First we have the concept of an
"inflictor" associated with a damage, which may or may not exist.  Drowning
damage, for example, does not have an inflictor.  Inflictor could be a grenade
entity, a satchel charge entity, a human grunt, or even the player himself (in
the case of selfgaussing).  It is the first argument to
``CBaseMonster::TakeDamage`` in ``dlls/combat.cpp``.

Suppose :math:`\mathbf{v}` is the player velocity and :math:`\mathbf{p}` the
player position.  If an inflictor with position
:math:`\mathbf{p}_\text{inflictor}` exists, then with

.. math:: \mathbf{d} = \mathbf{p} - \mathbf{p}_\text{inflictor} + \langle 0, 0, 10\rangle

we have

.. math:: \mathbf{v}' = \mathbf{v} +
          \begin{cases}
          \min(1000, 10\Delta\mathcal{H}) \mathbf{\hat{d}} & \text{if duckstate} = 2 \\
          \min(1000, 5\Delta\mathcal{H}) \mathbf{\hat{d}} & \text{otherwise}
          \end{cases}

We can immediately see that if the duckstate is 2 the change in velocity is
greater.  It is sad to see that the maximum possible boost given by a single
damage is 1000 ups and not infinite.


Object manoeuvre
----------------

Let :math:`V` the value of ``sv_maxvelocity``.  Define

.. math:: \operatorname{clip}(\mathbf{v}) := \left[ \min(v_x, V), \min(v_y, V), \min(v_z, V) \right]

which is basically ``PM_CheckVelocity``.  Assuming the player is not
accelerating, :math:`\lVert\mathbf{v}\rVert > E` and the use key is pressed
then with :math:`\mathbf{\tilde{v}}_0 = \mathbf{v}_0` the subsequence player
velocities :math:`\mathbf{v}_k` and object velocities :math:`\mathbf{u}_k` is
given by

.. math:: \begin{align*}
          \mathbf{v}_{k+1} &= (1 - k\tau) \operatorname{clip}(0.3\mathbf{\tilde{v}}_k) \\
          \mathbf{\tilde{v}}_{k+1} &= \mathbf{u}_k + \mathbf{v}_k \\
          \mathbf{u}_{k+1} &= (1 - k\tau) \operatorname{clip}(0.3\mathbf{\tilde{v}}_{k+1})
          \end{align*}

The physics of object boosting is well understood with trivial implementation.
A trickier technique is fast object manoeuvre, which is the act of "bringing"
an object with the player at extreme speed for a relatively long duration.

The general idea is to repeatedly activate ``+use`` for just one frame then
deactive it for subsequent :math:`n` frames while pulling an object.  Observe
that when ``+use`` is active the player velocity will be reduced significantly.
And yet, when ``+use`` is deactivated, the player velocity will be equal to the
object velocity, which may well be moving very fast.  The player will then
continue to experience friction.

One important note to make is that despite the player velocity being scaled
down by 0.3 when ``+use`` is active, the object velocity will actually increase
in magnitude.  An implication of this is that the object will gradually
overtake the player, until it goes out of the player's use radius.  To put it
another way, we say that the *mean* object speed is greater than the mean
player speed.  To control the mean player speed, :math:`n` must be adjusted.
If :math:`n` is too low or too high, the mean player speed will be very low.
Therefore there must exist an optimal :math:`n` at which the mean player speed
is maximised.

However, we do not often use the optimal :math:`n` when carrying out this
trick.  Instead, we would use the smallest possible :math:`n` so that the
object mean speed will be as high as possible while ensuring the object stays
within the use radius.  This means the object will hit an obstruction as soon
as possible, so that we can change direction as soon as that happens.


Gauss boost and quickgauss
--------------------------

Gauss is a very powerful weapon by virtue of its damage and recoil, which can
be exploited to change the player velocity significantly.  When the secondary
fire is shot, the new player velocity will become

.. math:: \mathbf{v}' = \mathbf{v} + 250t \langle \cos\vartheta, \sin\vartheta, 0\rangle \cos\varphi

where :math:`0.5 \le t \le 4` is the charging time in seconds.  The damage
produced is :math:`D = 50t`.

Starting with the later versions of GoldSrc Half-Life, and including all Steam
versions, there is a magnificent exploit that makes the weapon fire in
secondary mode as though :math:`t = 4`, but taking only half a second to charge
plus one cell.  This technique is called quickgauss, so named because it is
quick to produce the maximum possible boost and damage, rather than having to
charge for a full 4 seconds and consuming a lot of cells.  This can be achieved
by charging up the weapon, then save and reload the game before the weapon
completes its minimum 0.5s charge.  Upon reloading, the weapon will complete
the 0.5s charge followed by a quickgauss shot.


Gauss reflect boost
-------------------

In secondary mode, explosions happen at the points at which the gauss beam
reflects.  Moreover, gauss beams only reflect if the angle of incidence
:math:`\alpha` (the smallest angle between the incoming beam and the plane
normal) is greater than 60 degrees.  Let :math:`D` the initial damage.  When
the beam reflects for the first time, then a blast damage is produced with
:math:`D\cos\alpha`.  The damage is then reduced to :math:`D(1 - \cos\alpha)`.
So on so forth.

The maximum blast damage is obviously 100.  This is true when the damage from
the gauss beam is 200 and :math:`\alpha` is 60 degrees.  Suppose a player is
ducked and shoots the flat floor beneath him with :math:`\alpha = \pi/3`.  The
height of the source of gauss beam will be :math:`18 + 12 = 30` units above the
ground.  Therefore the horizontal distance from the point of reflection to the
player is :math:`90/\sqrt{3}`, and it can be shown that the health loss is
75-77 (assuming no armour).  Interestingly, the direction of boost is
vertically upward.

Another more dramatic type of reflect boost is the following: facing the wall
(might need to adjust the yaw angle slightly) with 60 degrees pitch and shoot
while *unducked*.  When the gauss is shot, the beam will reflect at
:math:`16\sqrt{3}` units below the gun position.  Hence the blast damage is
around 86-91.  It turns out that the reflected beam will directly hit the
player, dealing another 100 damage.  The combined damage is therefore 186-191,
giving 930-955 ups of vertical boost.  If the player DWJ before boosting, the
resultant vertical speed would be approximately 1198-1223 ups, which is
enormous.  Note that normal jumping will not work, it has to be DWJ.  This is
to prevent the normal jumping animation from playing.  Presumably, movement of
the legs in the jumping animation causes the beam to miss the player hitboxes.


Selfgauss
---------

Selfgauss happens when a gauss beam hits certain obstructions.  The beam will
shoot out of the player's body, thereby hitting the head from within.  In
normal situations the beam will ignore the player unless it has been reflected
at least once.  In this case the damage inflictor is the player himself,
therefore :math:`\mathbf{\hat{d}} = \langle 0,0,1\rangle`, thus the boost is
vertically upward.  If the player is ducked, the pitch angle must be
sufficiently negative for the beam to headshot the player.

.. image:: _static/selfgauss-1.png

There are two possibilities that trigger selfgauss.  If a beam enters an
obstruction and exits it, the distance between the point of entry and the point
of exit must be numerically greater than the damage of the beam.  This distance
is :math:`\color{blue}{\ell}` in the figure above.  For example, if the
distance is 100 units and the damage is 110, then selfgauss will not be
triggered.  In addition, the beam must be able to exit the obstruction.  Thus
selfgaussing does not work if the obstruction touches a wall with no gap in
between.  On the other hand, if the beam crosses at least one non-worldspawn
entities (such as ``func_wall``) aside from the first obstruction, the distance
between the point of entry into the first obstruction and the point of exit out
of the *last non-worldspawn entity* is now considered.

Unfortunately, in some occasions if the obstruction is very thick selfgauss may
not be triggered, even if the conditions described above are fulfilled.  The
exact reason remains a mystery.

Selfgaussing can be very powerful.  It is possible to obtain a 1000 ups upward
boost using just 67 damage.  However, if the pitch angle is too high or too
low, the beam might not hit the head, hence reducing the boost significantly.
