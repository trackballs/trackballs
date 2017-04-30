;;; The four seasons - Summer (lv 3)

(day)
(fog 0.01)
(fog-color 0.8 0.8 1.0)
(set-track-name "Summer")
(set-author "Mathias Broxvall")
(map-is-transparent #t)
(scale-oxygen 4.0)  ;; Let player be under water longer

(start-time 300)
(cond 
 ((= (difficulty) *easy*) (start-time 360))
 ((= (difficulty) *normal*) (start-time 300))
 ((= (difficulty) *hard*) (start-time 270))
 )
(set-start-position 199.5 197.5)
;;(set-start-position 234.5 170.5)

(define enemy1 (new-mr-black 200.5 201.5))
(cond
 ((= (difficulty) *easy*) (set-acceleration enemy1 3.0))
 ((= (difficulty) *normal*) (set-acceleration enemy1 4.0))
 ((= (difficulty) *hard*) (set-acceleration enemy1 5.0)))

(add-cactus 199 194 0.3)

(define pipes (multi-pipe '((201.0 199.5 -6.9) (208.0 199.5 -10.5) (230.0 199.5 -10.5)) 0.4 #f))
;;(map (lambda (pipe) (set-primary-color pipe 0.5 0.5 0.5 0.5)) pipes)
(set-primary-color (sign "Remember to breath" 1.0 15.0 0.0 212.5 199.5 -8.0) 1.0 1.0 0.4)
(add-flag 212 199 100 0 1.0) 

(diamond 234.5 193.5)
(add-flag 234 193 100 0 0.5) 

;; The bonus area
(add-modpill 241.5 208.5 *mod-extra-life* 30 -1)
(add-flag 241 207 100 1 0.1) 
(add-flag 239 208 100 1 0.1) 

(define speed nil)
(cond
 ((= (difficulty) *easy*) (set! speed 0.70))
 ((= (difficulty) *normal*) (set! speed 0.40))
 ((= (difficulty) *hard*) (set! speed 0.20))
)
(add-cyclic-platform 234 183 234 183 -11.0 -9.0 -0.0 speed)
(add-cyclic-platform 235 182 235 182 -10.9 -8.9 -0.5 speed)
(add-cyclic-platform 234 181 234 181 -10.8 -8.8 -1.0 speed)
(add-cyclic-platform 235 180 235 180 -10.7 -8.7 -1.5 speed)
(add-cyclic-platform 234 179 234 179 -10.6 -8.6 -2.0 speed)
(add-cyclic-platform 235 178 235 178 -10.5 -8.5 -2.5 speed)
(add-cyclic-platform 234 177 234 177 -10.4 -8.4 -3.0 speed)
(add-cyclic-platform 235 176 235 176 -10.3 -8.3 -3.5 speed)
(add-cyclic-platform 234 175 234 175 -10.2 -8.2 -4.0 speed)
(add-cyclic-platform 235 174 235 174 -10.1 -8.1 -4.5 speed)
(add-cyclic-platform 234 173 234 173 -10.0 -8.0 -5.0 speed)

(add-flag 235 172 100 1 0.1) 
(add-modpill 234.5 170.5 *mod-float* 60 60)

(define enemy2 (new-mr-black 225.5 171.5))
(cond
 ((= (difficulty) *easy*) (set-acceleration enemy2 6.0))
 ((= (difficulty) *normal*) (set-acceleration enemy2 8.0))
 ((= (difficulty) *hard*) (set-acceleration enemy2 10.0)))
(set-modtime enemy2 *mod-float* -1.)


(add-cactus 208 167 0.3)

;; Goal!
(add-goal 205 169 #t "fourSeasons_4")
(add-flag 203 168 100 1 0.1) 

