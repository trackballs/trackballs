;;; level 1
;; Note. This level is much more complex than the others since
;; I want to demonstrate the flexibility of what can be done with
;; the GUILE scripting language. If you are looking to create a level
;; yourself it is better to look at one of the other levels first.

(day)
(set-track-name "Test (Hexasoft)")
(set-author "Yannick Perret")
(fog)
(map-is-transparent #t)

(restart-time 60.0)

(set-start-position 252.5 252.5)
(start-time 150)
;(add-goal 252 250 #f "lv2") for debugging

;(add-modpill 252.5 253.5 *mod-jump* 30 -1)
(if (= (difficulty) *easy*) (add-modpill 252.5 253.5 *mod-spike* 30 -1))

;; Secret bonus level
(define bonus-goto "lv2")
(add-goal 253 203 #f "bonus")
(add-modpill 253.5 196.5 *mod-frozen* 5 (- 22 (* 3 (difficulty))))

;; This would create a forcefield and a switch turning it on/off
(if (= (difficulty) *hard*)
	(begin
	  (define ff (forcefield 251.5 250.5 0.0 
							 2.0 0.0 0.0 
							 0.5 *ff-kill*))
	  (switch 252.5 249.5 (lambda () (set-onoff ff #t)) (lambda () (set-onoff ff #f)))
	  ))


;(smart-trigger 252.5 249.5 1.0 
;			   (lambda()(sign "Entering" 1.0 0.0 2.0 252.5 249.5))  
;			   (lambda()(sign "Leaving" 1.0 0.0 2.0 252.5 249.5)))

;; This is the goal
(add-goal 219 204 #f "lv2")

(add-flag 252 232 250 0 1.0) ;; After the jump
;;(add-flag 241 245 150 1 0.1) ;; Where the black ball is
(add-flag 231 229 100 0 1.0)
(add-flag 224 212 100 1 0.1)
(add-flag 219 199 150 1 0.1)
(add-flag 224 198 250 1 0.1)


(define speed 0.2)
(cond
 ((= (difficulty) *easy*) (set! speed 0.10))
 ((= (difficulty) *normal*) (set! speed 0.20))
 ((= (difficulty) *hard*) (set! speed 0.30))
)

(add-spike 250.5 228.5 speed -0.00)
(add-spike 249.5 228.5 speed -0.20)
(add-spike 248.5 228.5 speed -0.40)
(add-spike 247.5 228.5 speed -0.60)
(add-spike 246.5 228.5 speed -0.80)
(add-spike 245.5 228.5 speed -1.00)

(add-flag 240 219 150 1 0.1)
(add-flag 232 218 250 1 0.1)

(set-primary-color (sign "Mr. Black" 1.0 15.0 0.0 241.5 245.5) 0.8 0.8 0.4)
(define mr-black (new-mr-black 240.5 244.5))


;;
;; place for my tests
;;

;; sidespike
(add-sidespike 251.5 241.5 speed 0.00 1)
(add-sidespike 253.5 241.5 speed 0.00 2)
(add-sidespike 248.5 245.5 speed 0.00 3)
(add-sidespike 246.5 245.5 speed 0.00 4)

(add-sidespike 248.5 239.5 speed 0.00 2)
(add-sidespike 247.5 238.5 speed 0.10 3)
(add-sidespike 246.5 239.5 speed 0.20 1)
(add-sidespike 246.5 241.5 speed 0.30 4)


;; a cactus test
(add-cactus 241 245 0.3)

;; a teleporter test
(add-teleport 252 243 252 239 0.3)

;; add a bird
(add-bird 253 249 253 240 0.5 0.9)
(set-flag (add-bird 253 253 240 240 0.5 0.9) *bird-constant-height* #t)

;; a color modifier
(add-colormodifier 0 252 249 0.1 0.9 1.0 0.0)
(add-colormodifier 1 252 248 0.0 1.0 1.2 0.1)
(add-colormodifier 2 252 247 0.5 1.0 1.5 0.2)
(add-colormodifier 3 252 246 0.0 0.5 2.0 0.3)

;; height modifiers
(add-heightmodifier 10 249 248 4.4 4.6 1.0 0.0)
(add-heightmodifier 11 249 248 4.4 4.6 1.1 0.1)
(add-heightmodifier 12 249 248 4.4 4.6 1.3 0.2 1)
(add-heightmodifier 13 249 248 4.4 4.6 1.2 0.3)
(add-heightmodifier 10 247 248 4.4 4.6 0.9 0.05)
(add-heightmodifier 11 247 248 4.4 4.6 1.1 0.15)
(add-heightmodifier 12 247 248 4.4 4.6 1.2 0.25)
(add-heightmodifier 13 247 248 4.4 4.6 1.3 0.35)


;;
;;  end of my tests
;;


(if (= (difficulty) *hard*) (set-modtime mr-black *mod-spike* -1.))
(set-acceleration mr-black 2.0)

;(set-onoff mr-black #f)
;(set-modtime mr-black *mod-spike* -1.)
;(set-primary-color mr-black 0.5 1.0 0.5)
;(set-specular-color mr-black 0.5 1.0 0.5)

(set-primary-color (sign "and his Minions" 1.0 15.0 0.0 233.5 245.5) 0.8 0.8 0.4)
(new-baby 233.5 245.5)
(new-baby 232.2 245.0)
(new-baby 233.5 246.0)
(new-baby 232.2 244.7)
(new-baby 232.8 246.5)
(new-baby 233.9 246.2)

(cond
 ((= (difficulty) *easy*) (set! speed 0.60))
 ((= (difficulty) *normal*) (set! speed 0.40))
 ((= (difficulty) *hard*) (set! speed 0.20))
)

(define plat1 (add-cyclic-platform 
 ;;; Positions
			   238 228 239 229
 ;;; Low, High, Time Offset, Speed
 3. 4. 0. speed)
)

(add-cyclic-platform 
 ;;; Positions
 236 228 237 229
 ;;; Low, High, Time Offset, Speed
 2. 4. 0.5 (+ 0.10 speed))

(add-cyclic-platform 
 ;;; Positions
 234 228 235 229
 ;;; Low, High, Time Offset, Speed
 3. 4. 0. speed)

(add-cyclic-platform 
 ;;; Positions
 230 225 231 229
 ;;; Low, High, Time Offset, Speed
 2. 3. 0.5 speed)

(diamond 242 229)

;;; modpill-types:
;;; *mod-speed* - "Speedball": Makes the player go faster
;;; *mod-jump* - "Jumpjets": Makes the player jump higher
;;; *mod-spike* - "Spikes": Damages other balls more in collisions. Increases friction on ice
;;; *mod-glass* - "Glassball": Makes ball more sensitive to crashes
;;; *mod-dizzy* - "Dizzy": Makes ball dizzy 
;;; *mod-frozen* - "Frozen": Player can't move ball as long as it is frozen

(add-modpill 231.5 212.5 *mod-spike* 25 40)
;(add-modpill 252.5 250.5 1 10 0)

;;;;;;;;;;;;;;;;;;;;;;;;,
;;; Note. These are some extra functions you can use

;; Calls your lambda expression when player is close to X,Y
;(trigger X Y R (lambda () <something to do>))

;; Sets the players position
;(set-ply-position X Y Z)

;; Sets the players velocity
;(set-ply-position X Y Z)

;; Gets position / velocity
;(ply-x) (ply-y) (ply-z)
;(ply-dx) (ply-dy) (ply-dz)

