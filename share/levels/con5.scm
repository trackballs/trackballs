;; new level

(day)
(set-track-name (_ "Snowbord"))
(set-author "Yannick Perret")
(fog)

(restart-time 60.0)

(set-start-position 252 247)
(start-time 150)

(if (> (difficulty) *easy*) ; MB. Only on medium and hard
  (add-modpill 222 226 *mod-spike* 30 -1))

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


(new-mr-black 231 249)
(new-mr-black 239 249)
(new-mr-black 251 244)
(new-mr-black 239 243)
(new-mr-black 224 252)
(new-mr-black 220 220)
(new-mr-black 219 219)
(new-mr-black 225 225)
(new-mr-black 224 224)
(new-mr-black 227 227)

;(fountain 224.5 221.5 -8.5 0.03 0.0 200.0)

(if (< (difficulty) *hard*) (set-cell-flag 239 248 237 246 *cell-sand* #t)) ; Sandpit on easy and normal
(if (= (difficulty) *easy*) (set-cell-flag 224 249 222 247 *cell-sand* #t)) ; Sandpit on easy

