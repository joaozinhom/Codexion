//the dongle is a passive shared resource, it does not talk to anyone.
//its mutex is possession itself: lock = taken, unlock = released, and it
//also guards the cooldown timestamp set on release