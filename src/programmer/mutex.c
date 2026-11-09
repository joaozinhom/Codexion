//the dongle mutex is possession itself: locking takes the dongle, unlocking
//releases it. it also guards the cooldown timestamp on release. the scheduler
//queue decides whose turn it is to try; each programmer alternates his own
//state