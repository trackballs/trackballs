;; SIX LEVELS
;; Scale the Mountain

;; in this misty, mysterious level, your goal is to get on top of the mountain. You will face many dangers on your way to the top. Don't forget the time limit!



(day)
(set-track-name (_ "Scale the Mountain"))
(set-author "Ulrik Enstad")
(cond
 ((= (difficulty) *easy*)
         (start-time 230)
 )
 ((= (difficulty) *normal*)
         (start-time 200)
 )
 ((= (difficulty) *hard*)
         (start-time 100)
 )
)
(set-start-position 176.5 175.5)
(map-is-transparent #t)
(fog)


;; fences at start
(pipe 178.5 167.2 -4.5 164.2 167.2 -4.5 0.08)
(pipe-connector 164.2 167.2 -4.5 0.08)
(pipe 164.2 167.2 -4.5 164.2 172.0 -4.5 0.08)
(pipe-connector 164.2 172.0 -4.5 0.08)

;; badguys
(set-primary-color (set-acceleration (new-mr-black 141.5 183.5) 1.6) 0.5 0.5 0.5)

(set-primary-color (set-acceleration (set-modtime
(new-mr-black 160.5 186.5) *mod-spike* -1.) 2.3) 0.8 0.0 0.0)

(set-primary-color (set-acceleration (new-mr-black 164.5 180.5) 1.6) 0.5 0.5 0.5)

(set-primary-color (set-acceleration (new-mr-black 168.5 186.5) 1.6) 0.5 0.5 0.5)

(set-primary-color (set-acceleration (set-modtime
(new-mr-black 151.5 191.5) *mod-spike* -1.) 2.3) 0.8 0.0 0.0)

(define kill-me (cond
 ((= (difficulty) *easy*)
          (set-modtime (new-mr-black 179.5 192.5) *mod-small* -1.)
 )
 ((= (difficulty) *normal*)
         (set-primary-color (set-acceleration (new-mr-black 179.5 192.0) 1.6) 0.5 0.5 0.5)
 )
 ((= (difficulty) *hard*)
         (set-primary-color (set-acceleration (new-mr-black 179.5 192.0) 1.6) 0.5 0.5 0.5)
 )
))
(on-event *death* kill-me (lambda (subject object)
(add-cyclic-platform 180 184 181 185 1.7 3.2 1. .5)
(add-cyclic-platform 180 182 181 183 3.2 4.7 4. .5)
(add-cyclic-platform 180 180 181 181 4.7 6.2 1. .5)
))

;; sign: kill me!
(sign (_ "Kill me!") 0.8 20.0 -1 179 192)

;; cactuses
(add-cactus 147 173 0.3)
(add-cactus 160 174 0.3)
(add-cactus 176 168 0.3)
(add-cactus 166 185 0.3)
(add-cactus 159 179 0.3)

;; cyclic platform
(add-cyclic-platform 140 185 142 187 -3.8 2.2 1. .9)

;; camera view
(trigger 144.5 186.5 1.5 (lambda () (camera-angle 1.0 0.0)))
(trigger 141.5 186.5 1.5 (lambda () (camera-angle 0.0 0.0)))
(trigger 183.5 177.5 0.5 (lambda () (camera-angle 0.0 0.0)))
(trigger 176.5 175.5 0.5 (lambda () (camera-angle 0.0 0.0)))
(trigger 181.0 187.0 1.0 (lambda () (camera-angle 2.0 0.0)))
(trigger 179.0 187.0 1.0 (lambda () (camera-angle 1.0 0.0)))
(trigger 179.0 187.0 1.0 (lambda () (camera-angle 1.0 0.0)))
(trigger 181.0 178.0 1.0 (lambda () (camera-angle 0.0 0.0)))
(trigger 181.0 181.0 1.0 (lambda () (camera-angle 2.0 0.0)))

;; isn't this a cool one?! try to roll over it right before it is absolutely horisontal and keep as close to the wall as possible.
(add-heightmodifier 10 160 185 2.7 4.2 0.2 0.00)
(add-heightmodifier 10 159 185 2.7 4.2 0.2 0.00)
(add-heightmodifier 10 158 185 2.7 4.2 0.2 0.00)
(add-heightmodifier 10 157 185 2.7 4.2 0.2 0.00)

(add-heightmodifier 10 160 186 2.7 2.7 0.2 0.00)
(add-heightmodifier 10 159 186 2.7 2.7 0.2 0.00)
(add-heightmodifier 10 158 186 2.7 2.7 0.2 0.00)
(add-heightmodifier 10 157 186 2.7 2.7 0.2 0.00)

(add-heightmodifier 10 160 187 2.7 1.2 0.2 0.00)
(add-heightmodifier 10 159 187 2.7 1.2 0.2 0.00)
(add-heightmodifier 10 158 187 2.7 1.2 0.2 0.00)
(add-heightmodifier 10 157 187 2.7 1.2 0.2 0.00)

(add-heightmodifier 10 160 188 2.7 -0.3 0.2 0.00)
(add-heightmodifier 10 159 188 2.7 -0.3 0.2 0.00)
(add-heightmodifier 10 158 188 2.7 -0.3 0.2 0.00)
(add-heightmodifier 10 157 188 2.7 -0.3 0.2 0.00)

;; parallell pipes. Roll between them!
(pipe 173 178.2 6.5 170 178.2 6.5 0.08)
(pipe 173 178.8 6.5 170 178.8 6.5 0.08)

;; final pipe. Almost there!
(pipe 166.5 184.5 10.8 171.5 184.5 10.8 0.4)

;; finally in goal. Puh!
(add-goal 179 184 #t "sl3")

;; points
(add-flag 151 191 50 1 0.1)
(add-flag 155 191 50 1 0.1)
(add-flag 145 183 50 1 0.1)
(add-flag 162 181 50 1 0.1)
(add-flag 168 174 50 1 0.1)

(define timeflag1 (add-flag 153 191 0 1 0.1))
(time-on-death timeflag1 30)
(set-primary-color timeflag1 0.0 1.0 0.0)
(define timeflag2 (add-flag 153 184 0 1 0.1))
(time-on-death timeflag2 30)
(set-primary-color timeflag2 0.0 1.0 0.0)

(add-modpill 147.5 178.5 *mod-extra-life* 10 0)

(diamond 181 178)

;; hints to win level:
;; kill the marble to activate the cyclic platforms
;; roll carefully around the deep hole
