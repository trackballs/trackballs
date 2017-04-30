
(start-time 180)
(set-start-position 208.5 211.5)
(add-goal 208 218 #t "frg7")

(add-modpill 215.5 210.5 *mod-extra-life* 10 0)
(sign "extra life: 800 points" 1 20 -1 215 210)
(add-flag 215 210 -800 0 0.3)

(add-modpill 215.5 213.5 *mod-large* 30 0)
(sign "giant-package half a minute: 200 points" 1 20 -1 215 213)
(add-flag 215 213 -200 0 0.3)

(add-modpill 215.5 216.5 *mod-speed* 60 0)
(sign "speed-pill one minute: 200 points" 1 20 -1 215 216)
(add-flag 215 216 -200 0 0.3)

(add-modpill 215.5 219.5 *mod-nitro* 120 0)
(sign "nitro-speed two minutes: 1000 points" 1 20 -1 215 219)
(add-flag 215 219 -1000 0 0.3)

(jump 0.1)
