;; SIX LEVELS
;; The Longest Pipe in the World

;; This level contains use of the newest guile functions, such as the "on-event" function and the "time-on-death" function. A surprising level.



(day)
(set-track-name (_ "The Longest Pipe in the World"))
(set-author "Ulrik Enstad")
(set-start-position 234 70)
(cond
 ((= (difficulty) *easy*)
         (start-time 300)
 )
 ((= (difficulty) *normal*)
         (start-time 260)
 )
 ((= (difficulty) *hard*)
         (start-time 200)
 )
)
(restart-time 0)


;; cyclic platforms
(add-cyclic-platform 232 78 233 79 3.0 7.0 1. .5)
(add-cyclic-platform 209 80 210 81 7.0 9.5 1. .5)


;; modpills
(add-modpill 234.5 78.5 *mod-extra-life* 10 0)
(add-modpill 203.5 78.5 *mod-small* 90 0) ; use this to get into "longest pipe in the world"
(add-modpill 234.5 92.5 *mod-spike* 20 20) ; use this to murder "mr-blue"


;; enemies
(set-primary-color (set-acceleration (new-mr-black 225.5 80.5) 1.6) 0.5 0.5 0.5)

(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 209.5 78.5) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)

(define mr-blue (set-acceleration (set-modtime (set-primary-color (set-secondary-color (new-mr-black 225.5 86.5) 1.0 1.0 1.0) 0.0 0.3 1.0) *mod-speed* -1.) 8.0))
(set-modtime (set-acceleration (new-mr-black 213.5 90.5) 8.0) *mod-small* -1.)

(set-modtime (set-acceleration (new-mr-black 211.5 88.5) 8.0) *mod-small* -1.)

(set-modtime (set-acceleration (new-mr-black 203.5 86.5) 8.0) *mod-small* -1.)

(set-modtime (set-acceleration (new-mr-black 203.5 80.5) 8.0) *mod-small* -1.)

(set-primary-color (set-acceleration (set-modtime
(new-mr-black 248.5 57.5) *mod-spike* -1.) 2.3) 0.8 0.0 0.0)

;; how to remove the ice
(on-event *death* mr-blue (lambda (subject object)
(set-cell-flag 213 92 221 93 *cell-ice* #f)
))


;; longest pipe in the world
(map (lambda (p) (set-primary-color (set-wind p 8.0 8.0) 0.0 0.7 0.0 0.5))
     (multi-pipe '((225.5 91.5 10.8)
                  (225.6 91.5 9.7)
                  (233.5 89.5 9.7)
                  (233.5 80.0 9.7)
                  (233.0 79.5 8.2)
                  (222.0 79.5 7.2)
                  (221.5 78.5 5.7)
                  (221.5 77.5 3.2)
                  (221.5 74.5 3.2)
                  (221.5 72.5 2.2)
                  (221.5 69.0 2.2)
                  (222.0 68.5 2.2)
                  (235.5 68.5 2.2)
                  (240.5 68.5 2.2)
                  (242.5 66.5 2.1)
                  (242.5 63.5 2.0)
                  (240.5 61.5 1.9)
                  (237.5 61.5 1.8)
                  (235.5 63.5 1.7)
                  (235.5 66.5 1.6)
                  (237.5 68.5 1.5)
                  (240.5 68.5 1.4)
                  (242.5 66.5 1.3)
                  (242.5 63.5 1.2)
                  (240.5 61.5 1.1)
                  (237.5 61.5 1.0)
                  (235.5 63.5 0.9)
                  (235.5 66.5 0.8)
                  (237.5 68.5 0.7)
                  (240.5 68.5 0.6)
                  (242.5 66.5 0.5)
                  (242.5 63.5 0.4)) 0.18 #t))

;; magic-platform
(define magic-platform (animator 0.2 0.0 0.0 0.5 3.0 *animator-stop* (lambda (v) (set-cell-heights 253 59 254 60 v v v v))))
(set-primary-color (switch 253.5 59.5 (lambda () (set-animator-direction magic-platform -0.2))  (lambda () (set-animator-direction magic-platform 0.2))) 1.0 0.0 0.0)

;; finally, the goal
(add-goal 254 62 #f "sl4")

;; points
(add-flag 210 79 50 #t 0.1)
(add-flag 210 78 50 #t 0.1)
(add-flag 209 78 50 #t 0.1)
(add-flag 209 79 50 #t 0.1)
(add-flag 202 89 50 #t 0.1)
(add-flag 202 88 50 #t 0.1)
(add-flag 201 88 50 #t 0.1)
(set-primary-color (add-flag 201 89 50 #t 0.1) 1.0 1.0 0.0)

(define timeflag1 (add-flag 229 91 0 #t 0.1))
(time-on-death timeflag1 30)
(set-primary-color timeflag1 0.0 1.0 0.0)

(define timeflag2 (add-flag 229 92 0 #t 0.1))
(time-on-death timeflag2 30)
(set-primary-color timeflag2 0.0 1.0 0.0)

(diamond 234.5 85.5)

;; sign
(sign (_ "Smash some badguys and see what happens") 0.5 20 -1 224.5 91.5)

;; hints to win level
;; kill the blue marble to remove the ice
;; kill the small marbles before grabbing the small ball modpill
;; use the small ball modpill to get into the world's longest pipe
