;;; lv2

(day)
(set-track-name "Acid")
(set-author "Mathias Broxvall")

(start-time 150) 
(set-start-position 251.5 251.5)

;; Mr. Black's territory "hard route"
(define mr-black (new-mr-black 250.5 242.5))
(cond
 ((= (difficulty) *easy*) (set-acceleration mr-black 3.0))
 ((= (difficulty) *normal*) (set-acceleration mr-black 4.0))
 ((= (difficulty) *hard*) (set-acceleration mr-black 5.0)))
(add-modpill 254.5 242.5 *mod-spike* 20 0)
(add-flag 250 242 150 1 0.2)

;; Area with spikes "easy route"
(define (set-colors s) (set-primary-color s 0.7 0.7 0.7) (set-secondary-color s 0.5 0.5 0.5))
(define speed 0.3)
(cond
 ((= (difficulty) *easy*) (set! speed 0.30))
 ((= (difficulty) *normal*) (set! speed 0.50))
 ((= (difficulty) *hard*) (set! speed 0.70)))
(set-colors (add-spike 243.5 252.5 speed -0.00))
(set-colors (add-spike 243.5 251.5 speed -0.00))
(set-colors (add-spike 243.5 250.5 speed -0.00))
(set-colors (add-spike 243.5 249.5 speed -0.00))

(set-colors (add-spike 242.5 252.5 speed -0.00))
(set-colors (add-spike 242.5 251.5 speed -0.00))
(set-colors (add-spike 242.5 250.5 speed -0.00))
(set-colors (add-spike 242.5 249.5 speed -0.00))

(set-colors (add-spike 241.5 252.5 speed -0.00))
(set-colors (add-spike 241.5 251.5 speed -0.00))
(set-colors (add-spike 241.5 250.5 speed -0.00))
(set-colors (add-spike 241.5 249.5 speed -0.00))

(define pipe0 (pipe 240.5 252.5 -1.0 236.5 252.5 -1.0 0.5))
;(set-primary-color pipe0 0.5 1.0 0.5 0.5)
(add-flag 235 253 25 1 0.1)
(add-flag 234 253 25 1 0.1)
(add-flag 233 253 25 1 0.1)
(add-flag 235 252 25 1 0.1)
(add-flag 234 252 25 1 0.1)
(add-flag 233 252 25 1 0.1)
(add-flag 235 251 25 1 0.1)
(add-flag 234 251 25 1 0.1)
(add-flag 233 251 25 1 0.1)

; new
(add-cactus 242 242 0.3)

;(add-flag 240 252 75 1 0.2)
;;(set-colors (add-spike 240.5 252.5 0.5 -0.00))
(set-colors (add-spike 240.5 251.5 speed -0.00))
(set-colors (add-spike 240.5 250.5 speed -0.00))
(set-colors (add-spike 240.5 249.5 speed -0.00))

;; Acid tunnel
(add-modpill 242.5 234.5 *mod-glass* (- 20 (* 5 (difficulty))) 30)
(add-flag 238 242 50 1 0.1)
(add-flag 235 241 50 1 0.1)

;; Jump over acid
(add-flag 230 234 100 0 0.6)
(if (= (difficulty) *easy*)
	(add-modpill 232.5 244.5 *mod-jump* 10 30))
(diamond 228.5 244.5)
(sign "Jump!" 1.0 0.0 0.0 230.5 235.5)
(add-flag 230 228 100 1 0.1)
(add-flag 230 225 250 1 0.1)

;; The mad elevator
(add-cyclic-platform 
 ;;; Positions
223 227 225 229 
 ;;; Low, High, Time Offset, Speed
 -10. -.5 0. 0.40)

;; Labyrinth of acid
(add-flag 223 243 100 1 0.1)
(new-mr-black 218.5 242.5)

;; A meeting point
(add-flag 217 228 50 1 0.1)

;; Labyrinth
(add-flag 222 217 50 1 0.1)
(add-flag 212 224 50 1 0.1)
(set-primary-color (add-flag 203 226 150 1 0.1) 0.9 0.8 0.4)
(set-primary-color (add-flag 207 216 150 1 0.1) 0.9 0.8 0.4)

;; Ladders
(add-cyclic-platform 216 204 216 205 -3.5 -2.0 0.00 0.30)
(add-cyclic-platform 217 204 217 205 -2.0 -0.5 0.50 0.40)
(add-cyclic-platform 218 204 218 205 -0.5  1.0 1.00 0.30)
(add-cyclic-platform 219 204 219 205  1.0  2.5 1.50 0.40)

;; Goal!
(add-goal 221 194 #f "lv3")

;;; *mod-speed* - "Speedball": Makes the player go faster
;;; *mod-jump* - "Jumpjets": Makes the player jump higher
;;; *mod-spike* - "Spikes": Damages other balls more in collisions. Increases friction on ice
;;; *mod-glass* - "Glassball": Makes ball immune to acid but more sensitive to crashes
;;; *mod-dizzy* - "Dizzy": Makes ball dizzy 
;;; *mod-frozen* - "Frozen": Player can't move ball as long as it is frozen
