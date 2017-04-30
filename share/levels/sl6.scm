;; SIX LEVELS
;; The Marble HQ

;; In the last level you have to infiltrate The Marble HQ where all the evil marbles were created. "Marwin, the Commander of Evil Marbles" is your target. You can find him in the highest tower of the HQ. Kill him to win the level set.            In this level you have to think.



(day)
(set-track-name "The Marble HQ")
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
(set-start-position 156.5 65.5)
(map-is-transparent #t)


;; turning off the HQ entry Forcefield
(trigger 149.0 73.0 1.0
 (lambda ()
  (set-cell-flag 144 72 149 77 *cell-ice* #t)
 )
)



(define mr-mini1 (set-modtime (new-mr-black 147 75) *mod-small* -1.))



(on-event *death* mr-mini1
 (lambda (subject object)
  (set-cell-flag 144 72 149 77 *cell-ice* #f)
  (pipe 145.5 77.9 2.4 145.5 84.1 2.8 0.5)
 )
)



(add-cyclic-platform 144 89 145 90 2.3 4.8 1. .4)



(define ff1
 (set-primary-color
  (forcefield 155 75 0.0 3.0 0.0 0.0 1.5 *ff-bounce*)
 0.0 1.0 0.0)
)



(set-primary-color
 (switch 156.5 78.5 
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
  (forcefield 173 78 0.0 0.0 2.0 0.0 1.5 *ff-bounce*)
 0.0 1.0 0.0)
)



(set-primary-color
 (switch 174 82
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
      (new-mr-black 171 75)
     1.0 1.0 1.0)
    0.7 0.7 0.7)
   200.0)
  *mod-spike* -1.)
 12.0 )
)



(diamond 186.5 75.5)



(define ff3
 (set-primary-color
  (forcefield 197 65 0.0 0.0 2.0 0.0 1.5 *ff-bounce*)
 0.0 1.0 0.0)
)



(set-primary-color
 (switch 201 71 
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
     (new-mr-black 186 63)
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
      (new-mr-black 192 70)
     1.0 1.0 1.0)
    0.7 0.7 0.7)
   200.0)
  *mod-spike* -1.)
 12.0 )
)



(set-primary-color (pipe 191.5 68.5 4.7 191.5 65.5 2.7 0.4) 0.3 0.3 0.3)
(set-primary-color (pipe-connector 191.5 65.5 2.7 0.4) 0.3 0.3 0.3)
(set-primary-color (pipe 191.5 65.5 2.7 191.5 55.5 2.7 0.4) 0.3 0.3 0.3)
(set-primary-color (pipe-connector 191.5 55.5 2.7 0.4) 0.3 0.3 0.3)
(set-primary-color (pipe 191.5 55.5 2.7 191.5 52.5 4.7 0.4) 0.3 0.3 0.3)



;; the part with sand, nitro and the bonus
(diamond 189.5 52.5)



(define mr-white
 (set-acceleration
  (set-modtime
   (set-horizon
    (set-secondary-color
     (set-primary-color
      (new-mr-black 201 45)
     1.0 1.0 1.0)
    0.7 0.7 0.7)
   200.0)
  *mod-spike* -1.)
 12.0 )
)



(sign "The Marble HQ - Do not Enter!" 0.8 20 -1 156.5 75)



(define ff4
 (set-primary-color
  (forcefield 204 51 0.0 1.0 0.0 0.0 1.5 *ff-bounce*)
 1.0 0.0 0.0)
)




(set-primary-color
 (switch 201.5 53.5
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
  (forcefield 209 55 0.0 3.0 0.0 0.0 1.5 *ff-bounce*)
 1.0 1.0 0.0)
)




(set-primary-color
 (switch 207.5 50.5
  (lambda ()
   (set-onoff ff5 #t)
  )
  (lambda ()
   (set-onoff ff5 #f)
  )
 )
1.0 1.0 0.0)




(set-primary-color (add-flag 204 47 500 1 0.1) 1.0 1.0 0.0)
(add-cactus 204 47 0.4)





(add-modpill 225.5 58.5 *mod-nitro* 15 15)




(define ff6
 (set-primary-color
  (forcefield 218 58 0.0 5.0 0.0 0.0 1.5 *ff-bounce*)
 0.0 1.0 1.0)
)




(set-primary-color
 (switch 221 60
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
      (new-mr-black 221 56)
     1.0 1.0 1.0)
    0.7 0.7 0.7)
   200.0)
  *mod-spike* -1.)
 12.0 )
)



(diamond 210.5 64.5)







(define ff9
 (set-primary-color
  (forcefield 233 97 0.0 2.0 0.0 0.0 1.5 *ff-bounce*)
 0.0 0.1 0.3)
)




(set-primary-color
 (switch 234.5 100.5
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


(sign "bonus" 0.8 20.0 -1 233 109)


(trigger 232.5 100.5 0.5
 (lambda ()
  (set-primary-color
   (pipe 221.5 100.5 4.7 228.5 108.5 4.7 0.4)
  0.1 0.1 0.1)
 )
)




(define mr-white
 (set-acceleration
  (set-modtime
   (set-horizon
    (set-secondary-color
     (set-primary-color
      (new-mr-black 240.5 128.5)
     1.0 1.0 1.0)
    0.7 0.7 0.7)
   200.0)
  *mod-spike* -1.)
 12.0 )
)



(define ff7
 (set-primary-color
  (forcefield 240 126 0.0 1.0 0.0 0.0 2.5 *ff-bounce*)
 1.0 0.0 1.0)
)




(set-primary-color
 (switch 232.5 115.5
  (lambda ()
   (set-onoff ff7 #t)
  )
  (lambda ()
   (set-onoff ff7 #f)
  )
 )
1.0 0.0 1.0)





(set-primary-color
 (switch 240.5 128.5
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
 (add-flag 243 138 500 1 0.1)
1.0 1.0 0.0)




(trigger 233.5 113.5 1.5
 (lambda ()
  (camera-angle 0.0 1.0)
 )
)



(trigger 240.5 134.5 0.5
 (lambda ()
  (camera-angle 0.0 0.0)
 )
)




(trigger 210.5 64.5 0.5
 (lambda ()
  (camera-angle 0.0 0.0)
 )
)

;; (bonus end)





(diamond 234 89)




(define ff10
 (set-primary-color
  (forcefield 244 79 0.0 0.0 2.0 0.0 1.5 *ff-bounce*)
 1.0 0.5 0.0)
)




(set-primary-color
 (switch 253.5 68.5
  (lambda ()
   (set-onoff ff10 #t)
  )
  (lambda ()
   (set-onoff ff10 #f)
  )
 )
1.0 0.5 0.0)





(add-modpill 254 40 *mod-extra-life* 10 0)





(set-primary-color 
 (pipe 254.5 68.1 4.7 254.5 48.5 4.7 0.4)
0.0 0.5 0.0 0.5)





(define magic-platform
 (animator 0.2 0.0 0.0 3.3 8.7 *animator-stop*
  (lambda (v)
   (set-cell-heights 249 75 249 75 v v v v)
  )
 )
)

(switch 249.5 75.5
 (lambda ()
  (set-animator-direction magic-platform -0.08)
 )
 (lambda ()
  (set-animator-direction magic-platform 0.08)
 )
)





(set-primary-color
 (pipe 249.5 75.9 9.2 249.5 90.5 9.2 0.4)
0.5 0.5 0.0 0.5)





(sign "Commander's Hall" 0.8 20 -1 249.5 90.5)






;; The fight against "Marwin the commander of evil marbles"


(trigger 249.5 109.5 1.5
 (lambda ()
  (set-cell-heights 248 103 250 107 -8.0 -8.0 -8.0 -8.0)
  (thick-fog)
  (fog-color 0.0 0.0 0.0)
 )
)


(diamond 249.5 109.5)


(trigger 249.5 107.5 0.3 
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
      (new-mr-black 249.5 115.5)
     "texture4.png")
    1.0 1.0 1.0)
   *mod-large* -1.)
  0.0)
 8.0)
)





(define the-trigger
 (trigger 249.5 115.5 1.5
  (lambda ()
   (set-horizon marwin 50.0)
  )
 )
)



(on-event *death* marwin
 (lambda (subject object)
  (play-effect "kill_death.wav")
  (set-primary-color (pipe 249.5 116.5 10.7 249.5 121.5 10.7 0.4) 0.5 0.5 0.5 0.5)
  (day)
 )
)



(add-goal 249 122 #f "")







;; If you're stuck in this level, you may find it helpful to read these hints:
;; 1. The white marbles, (mr. White) always know exactly where at the level you are (because of their huge horizon). This makes it hard to get past them without trapping them in a way. Make them go into the small chambers with forcefield doors, and lock them inside the chambers by pulling the switches. The switches and the forcefields which belong to each other have the same colors.

;; 2. This is how to get past the cactuses: Before you get to the cactus area, there is a mr. White locked behind a light blue forcefield. You have to gab the nitro mod pill, pull the light blue switch and race as fast as you can with Mr. White right behind you. Jump over the ledge. Make him destroy the cactuses and finally, trap him.

;; 3. Kill Marwin by making him roll over the spike. His dead triggers a pipe to the goal.
