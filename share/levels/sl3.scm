;; SIX LEVELS
;; The Longest Pipe in the World

;; This level contains use of the newest guile functions, such as the "on-event" function and the "time-on-death" function. A surprising level.



(day)
(set-track-name "The Longest Pipe in the World")
(set-author "Ulrik Enstad")
(set-start-position 234.5 70.5)
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
(map-is-transparent #t)
(restart-time 0)


;; cyclic platforms
(add-cyclic-platform 232 78 233 79 3.0 7.0 1. .5)
(add-cyclic-platform 209 80 210 81 7.0 9.5 1. .5)


;; modpills
(add-modpill 235 79 *mod-extra-life* 10 0)
(add-modpill 204 79 *mod-small* 90 0) ; use this to get into "longest pipe in the world"
(add-modpill 235 93 *mod-spike* 20 20) ; use this to murder "mr-blue"


;; enemies
(set-primary-color (set-acceleration (new-mr-black 226 81) 1.6) 0.5 0.5 0.5)

(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 210 79) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)

(define mr-blue (set-acceleration (set-modtime (set-primary-color (set-secondary-color (new-mr-black 226 87) 1.0 1.0 1.0) 0.0 0.3 1.0) *mod-speed* -1.) 8.0))
(set-modtime (set-acceleration (new-mr-black 214 91) 8.0) *mod-small* -1.)

(set-modtime (set-acceleration (new-mr-black 212 89) 8.0) *mod-small* -1.)

(set-modtime (set-acceleration (new-mr-black 204 87) 8.0) *mod-small* -1.)

(set-modtime (set-acceleration (new-mr-black 204 81) 8.0) *mod-small* -1.)

(set-primary-color (set-acceleration (set-modtime
(new-mr-black 249 58) *mod-spike* -1.) 2.3) 0.8 0.0 0.0)

;; how to remove the ice
(on-event *death* mr-blue (lambda (subject object)
(set-cell-flag 213 92 221 93 *cell-ice* #f)
))


;; longest pipe in the world
(set-primary-color (set-wind (pipe 226 92 10.8 226.1 92 9.7 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 226.1 92 9.7 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 226.1 92 9.7 234 90 9.7 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 234 90 9.7 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 234 90 9.7 234 80.5 9.7 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 234 80.5 9.7 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 234 80.5 9.7 233.5 80.0 8.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 233.5 80.0 8.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 233.5 80.0 8.2 222.5 80.0 7.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 222.5 80.0 7.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 222.5 80.0 7.2 222.0 79.0 5.7 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 222.0 79.0 5.7 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 222.0 79.0 5.7 222.0 78.0 3.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 222.0 78.0 3.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 222.0 78.0 3.2 222.0 75 3.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 222.0 75 3.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 222.0 75 3.2 222 73 2.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 222 73 2.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 222 73 2.2 222.0 69.5 2.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 222.0 69.5 2.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 222.0 69.5 2.2 222.5 69.0 2.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 222.5 69.0 2.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 222.5 69.0 2.2 236.0 69.0 2.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 236.0 69.0 2.2 241 69 2.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 241 69 2.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5) ; the helix starts here
(set-primary-color (set-wind (pipe 241 69 2.2 243 67 2.1 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 243 67 2.1 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 243 67 2.1 243 64 2.0 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 243 64 2.0 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 243 64 2.0 241 62 1.9 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 241 62 1.9 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 241 62 1.9 238 62 1.8 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 238 62 1.8 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 238 62 1.8 236 64 1.7 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 236 64 1.7 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 236 64 1.7 236 67 1.6 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 236 67 1.6 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 236 67 1.6 238 69 1.5 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 238 69 1.5 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 238.0 69.0 1.5 241 69 1.4 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 241 69 1.4 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 241 69 1.4 243 67 1.3 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 243 67 1.3 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 243 67 1.3 243 64 1.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 243 64 1.2 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 243 64 1.2 241 62 1.1 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 241 62 1.1 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 241 62 1.1 238 62 1.0 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 238 62 1.0 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 238 62 1.0 236 64 0.9 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 236 64 0.9 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 236 64 0.9 236 67 0.8 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 236 67 0.8 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 236 67 0.8 238 69 0.7 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 238 69 0.7 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 238 69 0.7 241 69 0.6 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 241 69 0.6 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 241 69 0.6 243 67 0.5 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe-connector 243 67 0.5 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5)
(set-primary-color (set-wind (pipe 243 67 0.5 243 64 0.4 0.18) 8.0 8.0) 0.0 0.7 0.0 0.5) ; long pipe, huh?



;; magic-platform
(define magic-platform (animator 0.2 0.0 0.0 0.5 3.0 *animator-stop* (lambda (v) (set-cell-heights 253 59 254 60 v v v v))))
(set-primary-color (switch 254 60 (lambda () (set-animator-direction magic-platform -0.2))  (lambda () (set-animator-direction magic-platform 0.2))) 1.0 0.0 0.0)

;; finally, the goal
(add-goal 254 62 #f "sl4")

;; points
(add-flag 210 79 50 1 0.1)
(add-flag 210 78 50 1 0.1)
(add-flag 209 78 50 1 0.1)
(add-flag 209 79 50 1 0.1)
(add-flag 202 89 50 1 0.1)
(add-flag 202 88 50 1 0.1)
(add-flag 201 88 50 1 0.1)
(set-primary-color (add-flag 201 89 50 1 0.1) 1.0 1.0 0.0)

(define timeflag1 (add-flag 229 91 0 1 0.1))
(time-on-death timeflag1 30)
(set-primary-color timeflag1 0.0 1.0 0.0)

(define timeflag2 (add-flag 229 92 0 1 0.1))
(time-on-death timeflag2 30)
(set-primary-color timeflag2 0.0 1.0 0.0)

(diamond 235 86)

;; sign
(sign "smash some badguys and see what happens" 0.5 20 -1 225 92)

;; hints to win level
;; kill the blue marble to remove the ice
;; kill the small marbles before grabbing the small ball modpill
;; use the small ball modpill to get into the world's longest pipe
