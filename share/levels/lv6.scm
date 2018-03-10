;;; level6 - cannon

(set-track-name (_ "Cannon")) (day)
(set-author "Mathias Broxvall")

(start-time 120)
(add-goal 254 241 #t "lv7")
(set-start-position 245 246)

(add-modpill 211 237 *mod-extra-life* 30 -1)

(define p1 
  (multi-pipe
   '((247 246 1.5) (250 246 1.5) (251 247 1.5)
   (251 249 1.5) (250 250 1.5) (249 250 1.5) 
   (241 250 -2.5) (239 250 -2.5)) 0.5 #t))
(map (lambda(p) (set-wind p 1.0 -4.0)) p1)
(set-wind (pipe 237 250 -2.5 232 250 -2.5 0.5) -2.0 -2.0)
(add-flag 238 254 250 #t 0.1)
(define dr-evil (new-mr-black 228 250))
(set-acceleration dr-evil (+ 2.0 (* 1.0 (difficulty))))

(define ff0 (forcefield 236.5 245.5 0.0 0.0 1.0 0.0 0.8 *ff-bounce*))
(set-wind (pipe 223 250 -2.5 219 250 0.0 0.5) 3.0 1.0)

(diamond 217.0 250.0)
(switch 211.0 245.0 
    (lambda() (set-onoff ff0 #t)) 
    (lambda()(set-onoff ff0 #f)))

(set-wind (pipe 243 246 1.5 239 246 -2.0 0.5) -1.0 -5.0)
(set-wind (pipe 235 241 -5.5 238 241 -5.5 0.5) 3.0 -3.0)
(define cannon
  (multi-pipe
   '((242 241 -5.5) (244 241 -5.5) (247 241 -1.0)) 0.5 #t))
(map (lambda(p) (set-wind p 15.0 -15.0)) cannon)

(add-flag 228 247 150 #t 0.1)

(add-cyclic-platform 244 236 244 236 -6.0 0.0 0.0 0.5)
(add-flag 247 237 50 #t 0.1)
(add-flag 246 237 50 #t 0.1)
(add-flag 245 237 50 #t 0.1)
(add-flag 247 236 50 #t 0.1)
(add-flag 246 236 50 #t 0.1)
(add-flag 245 236 50 #t 0.1)
(add-flag 247 235 50 #t 0.1)
(add-flag 246 235 50 #t 0.1)
(add-flag 245 235 50 #t 0.1)

(define speed 0.2)
(cond
 ((= (difficulty) *easy*) (set! speed 0.10))
 ((= (difficulty) *normal*) (set! speed 0.20))
 ((= (difficulty) *hard*) (set! speed 0.30))
)
(add-spike 211 250 speed -0.00)
