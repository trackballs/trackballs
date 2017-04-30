;;; waterworld

(day)
(set-track-name "Water world")
(set-author "Mathias Broxvall")
(map-is-transparent #t)
(add-goal 188 191 #t "")

(start-time (- 180 (* 15 (difficulty)))) 
(set-start-position 234.5 229.5)
(fog 0.5)
(pipe 251.5 226.5 3.4 234.5 226.5 3.3 0.4)
(define fountain-1 (fountain 235.5 226.5 3.2 0.01 0.1 200.0))
(set-fountain-velocity fountain-1 -0.2 0.0 0.15)
(add-modpill 227.5 203.5 *mod-float* 30 45)
(add-modpill 218.5 203.5 *mod-extra-life* 30 -1)

(add-cactus 210 202 0.3)
(add-cactus 210 206 0.3)
(set-flag (add-bird 208 214 208 185 0.5 1.5) *bird-constant-height* #t)
(add-flag 207 195 150 1 0.1)
(add-flag 209 193 150 1 0.1)
(define mb0 (new-mr-black 198.5 191.5))
(if (= (difficulty) *hard*) (set-modtime mb0 *mod-spike* -1.))

(define waterAnimator (animator 1.0 0.0 0.0 -4.2 -3.1 *animator-stop* (lambda (v) (set-cell-water-heights 191 188 201 194 v v v v))))
(switch 192.5 193.5 (lambda () (set-animator-direction waterAnimator -0.2)) (lambda () (set-animator-direction waterAnimator 0.2)))

(add-modpill 199.5 189.5 *mod-float* (- 30 (* 8 (difficulty))) 45)
(add-cactus 202 195 0.3)
(add-cactus 202 187 0.3)
(add-flag 197 187 150 1 0.1)
(add-flag 197 195 150 1 0.1)

(add-cyclic-platform 203 191 207 191 -8.0 -3.0 0.0 0.60)

