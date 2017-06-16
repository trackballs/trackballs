;; SIX LEVELS
;; The Marble HQ

;; In the last level you have to infiltrate The Marble HQ where all the evil marbles were created. "Marwin, the Commander of Evil Marbles" is your target. You can find him in the highest tower of the HQ. Kill him to win the level set.            In this level you have to think.



(day)
(set-track-name (_ "The Marble HQ"))
(set-author "Ulrik Enstad")
(cond
 ((= (difficulty) *easy*)
         (start-time 570)
 )
 ((= (difficulty) *normal*)
         (start-time 470)
 )
 ((= (difficulty) *hard*)
         (start-time 370)
 )
)
(set-start-position 156 65)


;; turning off the HQ entry Forcefield
(define iced-once #f)
(trigger 147.5 72.5 2.0
 (lambda ()
  (if (not iced-once)
    (begin
      (set-cell-flag 144 72 149 77 *cell-ice* #t)
      (set! iced-once #t))
 )
))



(define mr-mini1 (set-modtime (new-mr-black 146.5 74.5) *mod-small* -1.))



(on-event *death* mr-mini1
 (lambda (subject object)
  (set-cell-flag 144 72 149 77 *cell-ice* #f)
  (pipe 145 77.4 2.4 145 83.6 2.8 0.5)
 )
)



(add-cyclic-platform 144 89 145 90 2.3 4.8 1. .4)



(define ff1
 (set-primary-color
  (forcefield 154.5 74.5 0.0 3.0 0.0 0.0 1.5 *ff-bounce*)
 0.0 1.0 0.0)
)



(set-primary-color
 (switch 156 78 
  (lambda ()
   (set-onoff ff1 #t)
  )
  (lambda ()
   (set-onoff ff1 #f)
  )
 )
0.0 1.0 0.0)






;; the part of the camp where you have to trap white marbles
(define ff2
 (set-primary-color
  (forcefield 172.5 77.5 0.0 0.0 2.0 0.0 1.5 *ff-bounce*)
 0.0 1.0 0.0)
)



(set-primary-color
 (switch 173.5 81.5
  (lambda ()
   (set-onoff ff2 #t)
  )
  (lambda ()
   (set-onoff ff2 #f)
  )
 )
0.0 1.0 0.0)



(define mr-white
 (set-acceleration
  (set-modtime
   (set-horizon
    (set-secondary-color
     (set-primary-color
      (new-mr-black 170.5 74.5)
     1.0 1.0 1.0)
    0.7 0.7 0.7)
   200.0)
  *mod-spike* -1.)
 12.0 )
)



(diamond 186 75)



(define ff3
 (set-primary-color
  (forcefield 196.5 64.5 0.0 0.0 2.0 0.0 1.5 *ff-bounce*)
 0.0 1.0 0.0)
)



(set-primary-color
 (switch 200.5 70.5 
  (lambda ()
   (set-onoff ff3 #t)
   (set-cell-heights 197 74 197 76 4.2 4.2 4.2 4.2)
  )
  (lambda ()
   (set-onoff ff3 #f)
   (set-cell-heights 197 74 197 76 5.5 5.5 5.5 5.5)
  )
 )
0.0 1.0 0.0)



(define mr-green
 (set-horizon
  (set-acceleration
   (set-primary-color
    (set-secondary-color
     (new-mr-black 185.5 62.5)
    0.0 0.3 1.0)
   0.0 0.5 0.0)
  6.5)
 3.0)
)



(define mr-white
 (set-acceleration
  (set-modtime
   (set-horizon
    (set-secondary-color
     (set-primary-color
      (new-mr-black 191.5 69.5)
     1.0 1.0 1.0)
    0.7 0.7 0.7)
   200.0)
  *mod-spike* -1.)
 12.0 )
)



(set-primary-color (pipe 190.5 68.0 4.7 191.0 65.0 2.7 0.4) 0.3 0.3 0.3)
(set-primary-color (pipe-connector 191.0 65.0 2.7 0.4) 0.3 0.3 0.3)
(set-primary-color (pipe 191.0 65.0 2.7 191.0 55.0 2.7 0.4) 0.3 0.3 0.3)
(set-primary-color (pipe-connector 191.0 55.0 2.7 0.4) 0.3 0.3 0.3)
(set-primary-color (pipe 191.0 55.0 2.7 191.0 52.0 4.7 0.4) 0.3 0.3 0.3)



;; the part with sand, nitro and the bonus
(diamond 189.0 52.0)



(define mr-white
 (set-acceleration
  (set-modtime
   (set-horizon
    (set-secondary-color
     (set-primary-color
      (new-mr-black 200.5 44.5)
     1.0 1.0 1.0)
    0.7 0.7 0.7)
   200.0)
  *mod-spike* -1.)
 12.0 )
)



(sign (_ "The Marble HQ - Do not Enter!") 0.8 20 -1 156.0 74.5)



(define ff4
 (set-primary-color
  (forcefield 203.5 50.5 0.0 1.0 0.0 0.0 1.5 *ff-bounce*)
 1.0 0.0 0.0)
)




(set-primary-color
 (switch 201 53
  (lambda ()
   (set-onoff ff4 #t)
  )
  (lambda ()
   (set-onoff ff4 #f)
  )
 )
1.0 0.0 0.0)




(define ff5
 (set-primary-color
  (forcefield 208.5 54.5 0.0 3.0 0.0 0.0 1.5 *ff-bounce*)
 1.0 1.0 0.0)
)




(set-primary-color
 (switch 207 50
  (lambda ()
   (set-onoff ff5 #t)
  )
  (lambda ()
   (set-onoff ff5 #f)
  )
 )
1.0 1.0 0.0)




(set-primary-color (add-flag 204 47 500 #t 0.1) 1.0 1.0 0.0)
(add-cactus 204 47 0.4)





(add-modpill 225 58 *mod-nitro* 15 15)




(define ff6
 (set-primary-color
  (forcefield 217.5 57.5 0.0 5.0 0.0 0.0 1.5 *ff-bounce*)
 0.0 1.0 1.0)
)




(set-primary-color
 (switch 220.5 59.5
  (lambda ()
   (set-onoff ff6 #t)
  )
  (lambda ()
   (set-onoff ff6 #f)
  )
 )
0.0 1.0 1.0)




(define mr-white
 (set-acceleration
  (set-modtime
   (set-horizon
    (set-secondary-color
     (set-primary-color
      (new-mr-black 220.5 55.5)
     1.0 1.0 1.0)
    0.7 0.7 0.7)
   200.0)
  *mod-spike* -1.)
 12.0 )
)



(diamond 210 64)







(define ff9
 (set-primary-color
  (forcefield 232.5 96.5 0.0 2.0 0.0 0.0 1.5 *ff-bounce*)
 0.0 0.1 0.3)
)




(set-primary-color
 (switch 234 100
  (lambda ()
   (set-onoff ff9 #t)
  )
  (lambda ()
   (set-onoff ff9 #f)
  )
 )
0.0 0.1 0.3)



(add-cactus 229 96 0.5)
(add-cactus 229 95 0.5)





;; (bonus)


(sign (_ "Bonus") 0.8 20.0 -1 232.5 108.5)


(trigger 232 100 0.5
 (lambda ()
  (set-primary-color
   (pipe 221 100 4.7 228 108 4.7 0.4)
  0.1 0.1 0.1)
 )
)




(define mr-white
 (set-acceleration
  (set-modtime
   (set-horizon
    (set-secondary-color
     (set-primary-color
      (new-mr-black 240 128)
     1.0 1.0 1.0)
    0.7 0.7 0.7)
   200.0)
  *mod-spike* -1.)
 12.0 )
)



(define ff7
 (set-primary-color
  (forcefield 239.5 125.5 0.0 1.0 0.0 0.0 2.5 *ff-bounce*)
 1.0 0.0 1.0)
)




(set-primary-color
 (switch 232. 115
  (lambda ()
   (set-onoff ff7 #t)
  )
  (lambda ()
   (set-onoff ff7 #f)
  )
 )
1.0 0.0 1.0)





(set-primary-color
 (switch 240 128
  (lambda ()
   (set-onoff ff7 #t)
  )
  (lambda ()
   (set-onoff ff7 #f)
  )
 )
1.0 0.0 1.0)



(add-teleport 246 141 221 99 0.3)




(set-primary-color
 (add-flag 243 138 500 #t 0.1)
1.0 1.0 0.0)




(trigger 233 113 1.5
 (lambda ()
  (camera-angle 0.0 1.0)
 )
)



(trigger 240 134 0.5
 (lambda ()
  (camera-angle 0.0 0.0)
 )
)




(trigger 210 64 0.5
 (lambda ()
  (camera-angle 0.0 0.0)
 )
)

;; (bonus end)





(diamond 233.5 88.5)




(define ff10
 (set-primary-color
  (forcefield 243.5 78.5 0.0 0.0 2.0 0.0 1.5 *ff-bounce*)
 1.0 0.5 0.0)
)



(define magic-platform
 (animator 0.2 0.0 0.0 3.3 8.7 *animator-stop*
  (lambda (v)
   (set-cell-heights 249 75 249 75 v v v v)
  )
 )
)

(set-primary-color
 (switch 253 68
  (lambda ()
   (set-onoff ff10 #t)
  )
  (lambda ()
   (set-onoff ff10 #f)
   ; reset magic platform
   (set-animator-direction magic-platform -0.08)
  )
 )
1.0 0.5 0.0)





(add-modpill 253.5 39.5 *mod-extra-life* 10 0)





(set-primary-color 
 (pipe 254 67.6 4.7 254 48.0 4.7 0.4)
0.0 0.5 0.0 0.5)






(switch 249 75
 (lambda ()
  (set-animator-direction magic-platform -0.08)
 )
 (lambda ()
  (set-animator-direction magic-platform 0.08)
 )
)





(set-primary-color
 (pipe 249 75 9.2 249 90 9.2 0.4)
0.5 0.5 0.0 0.5)





(sign (_ "Commander's Hall") 0.8 20 -1 249 90)






;; The fight against "Marwin the commander of evil marbles"


(trigger 249 109 1.5
 (lambda ()
  (set-cell-heights 248 103 250 107 -8.0 -8.0 -8.0 -8.0)
  (thick-fog)
  (fog-color 0.0 0.0 0.0)
 )
)


(diamond 249 109)


(trigger 249 107 0.3 
 (lambda ()
  (play-effect "evillaugh.wav")
 )
)


(add-spike 249.5 112.5 .3 0.00)




(define marwin
 (set-acceleration
  (set-horizon
   (set-modtime
    (set-primary-color 
     (set-texture
      (new-mr-black 249 115)
     "texture4.png")
    1.0 1.0 1.0)
   *mod-large* -1.)
  0.0)
 8.0)
)





(define the-trigger
 (trigger 249 115 1.5
  (lambda ()
   (set-horizon marwin 50.0)
  )
 )
)



(on-event *death* marwin
 (lambda (subject object)
  (play-effect "kill_death.wav")
  (set-primary-color (pipe 249 116 10.7 249 121 10.7 0.4) 0.5 0.5 0.5 0.5)
  (day)
 )
)



(add-goal 249 122 #f "")







;; If you're stuck in this level, you may find it helpful to read these hints:
;; 1. The white marbles, (mr. White) always know exactly where at the level you are (because of their huge horizon). This makes it hard to get past them without trapping them in a way. Make them go into the small chambers with forcefield doors, and lock them inside the chambers by pulling the switches. The switches and the forcefields which belong to each other have the same colors.

;; 2. This is how to get past the cactuses: Before you get to the cactus area, there is a mr. White locked behind a light blue forcefield. You have to gab the nitro mod pill, pull the light blue switch and race as fast as you can with Mr. White right behind you. Jump over the ledge. Make him destroy the cactuses and finally, trap him.

;; 3. Kill Marwin by making him roll over the spike. His dead triggers a pipe to the goal.
