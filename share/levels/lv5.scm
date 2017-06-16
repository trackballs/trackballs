;;; lv5.scm

(day)                           ;; Sets daylight for this level. 
(set-track-name (_ "Jump"))         ;; The name of the level
(set-author "Mathias BroxvalL") ;;
(start-time 120)                ;; We have two minutes to complete level
(set-start-position 251 251) ;; Where the player appears
(add-goal 200 222 #t "lv6")     ;; Where we should go (200,200) and which level "lv1" to
                                ;; play when we are finished.
(fog 0.2)
(fog-color 0.7 0.7 0.9)
(define bonus-goto "lv6")
(add-goal 206 224 #t "bonus")

(diamond 226 239)

(if (= (difficulty) *hard*)
  (set-acceleration (new-mr-black 251 239) 2.0))
(if (= (difficulty) *easy*)
  (add-modpill 251 239 *mod-spike* 20 30))

(add-flag 251 238 150 #t 0.1)
(add-flag 234 239 250 #t 0.1)
(add-flag 217 239 100 #f 2.5)
(add-flag 203 222 100 #f 1.5)
(add-flag 208 239 50 #t 0.1)

; new
(set-flag (add-bird 252 240 234 240 0.5 0.9) *bird-constant-height* #f)
(set-flag (add-bird 252 239 234 239 0.5 0.55) *bird-constant-height* #f)
(set-flag (add-bird 252 238 234 238 0.5 1.45) *bird-constant-height* #f)

(if (= (difficulty) *hard*)
  (new-mr-black 213.5 235.5))
(if (>= (difficulty) *normal*)
  (new-mr-black 213.5 232.5))
(if (>= (difficulty) *normal*)
  (new-mr-black 213.5 229.5))


