(start-time 120)
(set-start-position 236 236)
(add-goal 250 236 #t "frg4")

(add-modpill 243 237 *mod-speed* 120 0)
(sign (_ "Speed-pill 2 minutes: 300 points") 1 20 -1 242.5 236.5)
(add-flag 243 237 -300 #f 0.3)

(add-modpill 243 235 *mod-extra-life* 10 0)
(sign (_ "Extra Life: 800 points") 1 20 -1 242.5 234.5)
(add-flag 243 235 -800 #f 0.3)

