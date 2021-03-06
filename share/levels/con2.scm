;;; level kw1
;;; Note. This is meant to be an adult level.

(day)
(set-track-name (_ "Stairway to Heaven"))
(set-author "Keith Winston")

(set-start-position 252 252)
(start-time 240) 

(add-goal 219 219 #f "con3")

(add-flag 246 239 500 #t 0.1) 
(add-flag 252 239 100 #t 0.5)
(add-flag 248 244 100 #t 0.1)

(add-modpill 236 250 *mod-spike* 25 40)

(define mr-black (new-mr-black 224 240))
(if (= (difficulty) *hard*) (set-modtime mr-black *mod-spike* -1.))
(set-acceleration mr-black 2.0)

;;; modpill-types:
;;; *mod-speed* - "Speedball": Makes the player go faster
;;; *mod-jump* - "Jumpjets": Makes the player jump higher
;;; *mod-spike* - "Spikes": Damages other balls more in collisions. Increases friction on ice
;;; *mod-glass* - "Glassball": Makes ball more sensitive to crashes
;;; *mod-dizzy* - "Dizzy": Makes ball dizzy 
;;; *mod-frozen* - "Frozen": Player can't move ball as long as it is frozen

;(add-modpill 231.5 212.5 *mod-spike* 25 40)
;(add-modpill 252.5 250.5 1 10 0)


