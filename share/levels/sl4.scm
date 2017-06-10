;; SIX LEVELS
;; Hurry up!

;; you have to be really quick to win this level. You don't get any extra time when you die. The level is built a bit like a slide, with sharp turns, lots of obstacles and jumps. And remember, speed is the thing!



(day)
(set-track-name (_ "Hurry up!"))
(set-author "Ulrik Enstad")
(cond
 ((= (difficulty) *easy*)
         (start-time 150)
 )
 ((= (difficulty) *normal*)
         (start-time 100)
 )
 ((= (difficulty) *hard*)
         (start-time 90)
 )
)
(set-start-position 238.5 238.5)

(add-modpill 217.5 213.5 *mod-large* 20 20)

(set-primary-color (set-acceleration (new-mr-black 215.5 202.5) 1.6) 0.5 0.5 0.5)
(set-primary-color (set-acceleration (new-mr-black 221.5 202.5) 1.6) 0.5 0.5 0.5)

(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 221.5 198.5) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 215.5 198.5) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)

(set-acceleration (set-modtime (set-primary-color (set-secondary-color (new-mr-black 215.5 194.5) 1.0 1.0 1.0) 0.0 0.3 1.0) *mod-speed* -1.) 8.0)
(set-acceleration (set-modtime (set-primary-color (set-secondary-color (new-mr-black 221.5 194.5) 1.0 1.0 1.0) 0.0 0.3 1.0) *mod-speed* -1.) 8.0)

(pipe 210.0 148.1 -0.6 202.0 148.1 -0.6 0.08)
(pipe 210.0 146.9 -0.6 202.0 146.9 -0.6 0.08)

(pipe 175.5 147.5 0.2 174.5 147.5 0.2 0.4)
(pipe-connector 174.5 147.5 0.2 0.4)
(pipe 174.5 147.5 0.2 172.5 147.5 -3.3 0.4)
(pipe-connector 172.5 147.5 -3.3 0.4)
(pipe 172.5 147.5 -3.3 169.5 147.5 -3.3 0.4)

(smart-trigger 141.5 147.5 1.5 (lambda () (set-modtime (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 144.5 147.5) 1.0 1.0 1.0) 0.5 1.0 0.0) 8.0) *mod-large* -1.)) (lambda () (day)))

(set-modtime (set-primary-color (set-acceleration (set-modtime
(new-mr-black 80.5 127.5) *mod-spike* -1.) 2.3) 0.8 0.0 0.0) *mod-float* -1.)

(add-modpill 102.5 147.5 *mod-float* 20 20)

(add-goal 78 9 #f "sl5")

(restart-time 0)

;; hints to win level:
;; short cut/easier way: grab the big ball modpill roll between the parallell pipes
;; short cut (but very difficult): jump up on the edge of the platform where the big ball appairs and continue on the grey way
