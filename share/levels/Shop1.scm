(start-time 120)
(set-start-position 236.5 236.5)
(add-goal 250 236 #t "frg4")

(add-modpill 243.5 237.5 *mod-speed* 120 0)
(sign "Speed-pill 2 minutes: 300 points" 1 20 -1 243 237)
(add-flag 243 237 -300 0 0.3)

(add-modpill 243.5 235.5 *mod-extra-life* 10 0)
(sign "Extra Life: 800 points" 1 20 -1 243 235)
(add-flag 243 235 -800 0 0.3)

