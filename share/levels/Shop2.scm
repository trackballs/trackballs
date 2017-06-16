
(start-time 180)
(set-start-position 208 211)
(add-goal 208 218 #t "frg7")

(add-modpill 215 210 *mod-extra-life* 10 0)
(sign (_ "Extra life: 800 points") 1 20 -1 214.5 209.5)
(add-flag 215 210 -800 #f 0.3)

(add-modpill 215 213 *mod-large* 30 0)
(sign (_ "Giant-package half a minute: 200 points") 1 20 -1 214.5 212.5)
(add-flag 215 213 -200 #f 0.3)

(add-modpill 215 216 *mod-speed* 60 0)
(sign (_ "Speed-pill one minute: 200 points") 1 20 -1 214.5 215.5)
(add-flag 215 216 -200 #f 0.3)

(add-modpill 215 219 *mod-nitro* 120 0)
(sign (_ "Nitro-speed two minutes: 1000 points") 1 20 -1 214.5 218.5)
(add-flag 215 219 -1000 #f 0.3)

(jump 0.1)
