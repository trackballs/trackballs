;; new level

(day)
(set-track-name "Snowbord")
(set-author "Yannick Perret")
(fog)
(map-is-transparent #t)

(restart-time 60.0)

(set-start-position 252.5 247.5)
(start-time 150)

(if (> (difficulty) *easy*) ; MB. Only on medium and hard
	(add-modpill 222.5 226.5 *mod-spike* 30 -1))

(add-goal 219 215 #t "")


(add-cactus 249 251 0.3)
(add-cactus 248 245 0.3)
(add-cactus 242 250 0.3)
(add-cactus 240 243 0.3)
(add-cactus 236 246 0.3)
(add-cactus 232 250 0.3)
(add-cactus 229 248 0.3)
(add-cactus 225 245 0.3)
(add-cactus 220 250 0.3)
(add-cactus 220 243 0.3)


(add-teleport 219 245 223 228 0.4)


(new-mr-black 231.5 249.5)
(new-mr-black 239.5 249.5)
(new-mr-black 251.5 244.5)
(new-mr-black 239.5 243.5)
(new-mr-black 224.5 252.5)
(new-mr-black 220.5 220.5)
(new-mr-black 219.5 219.5)
(new-mr-black 225.5 225.5)
(new-mr-black 224.5 224.5)
(new-mr-black 227.5 227.5)

;(fountain 224.5 221.5 -8.5 0.03 0.0 200.0)

(if (< (difficulty) *hard*) (set-cell-flag 239 248 237 246 *cell-sand* #t)) ; Sandpit on easy and normal
(if (= (difficulty) *easy*) (set-cell-flag 224 249 222 247 *cell-sand* #t)) ; Sandpit on easy

