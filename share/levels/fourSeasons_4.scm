;; fourSeasons - Autumn (level 4)

(day)
(fog 0.3)
(fog-color 0.5 0.5 0.5)
(rain 0.6)
(set-track-name (_ "Autumn"))
(set-author "Mathias Broxvall")

(start-time 180) 
(set-start-position 199 199)
(add-goal 199 207 #f "")

(define enemy-1 (new-mr-black 199 186))
(if (= (difficulty) *hard*) (set-modtime enemy-1 *mod-large* -1.))
(set-acceleration enemy-1 2.5)

(add-flag 199 186 100 #t 0.1) 
(add-flag 197 175 25 #t 0.1) 
(add-flag 197 179 25 #t 0.1) 
(add-flag 201 179 25 #t 0.1) 
(add-flag 201 175 25 #t 0.1) 

(define sw-1 (switch 199 177 (lambda()(set-onoff ff-1 #t)) (lambda()(set-onoff ff-1 #f))))
(define ff-1 (forcefield 201.4 198 0.0 0.0 2.0 0.0 0.8 *ff-bounce*))
(set-primary-color ff-1 0.2 0.2 1.0)

(define spike-speed 0.2)
(cond
 ((= (difficulty) *easy*) (set! spike-speed 0.20))
 ((= (difficulty) *normal*) (set! spike-speed 0.30))
 ((= (difficulty) *hard*) (set! spike-speed 0.40))
)
(add-spike 202 198.9 spike-speed -0.00)
(add-spike 203 199.1 spike-speed -0.50)
(add-spike 204 199.1 spike-speed -1.00)
(add-spike 205 198.9 spike-speed -1.50)
(add-spike 206 199.1 spike-speed -2.00)

(define sw-2 (switch 214 200 (lambda()(set-onoff ff-2 #t)) (lambda()(set-onoff ff-2 #f))))
(define ff-2 (forcefield 196.6 198 0.0 0.0 2.0 0.0 0.8 *ff-bounce*))
(set-primary-color ff-2 0.2 0.2 1.0)

(add-flag 193 204 25 #t 0.1) 
(add-flag 191 204 25 #t 0.1) 
(add-flag 191 202 25 #t 0.1) 
(add-flag 193 202 25 #t 0.1) 

(define sw-3 (switch 192 203 (lambda()(set-onoff ff-3 #t)) (lambda()(set-onoff ff-3 #f))))
(define ff-3 (forcefield 198 201.4 0.0 2.0 0.0 0.0 0.8 *ff-bounce*))
(set-primary-color ff-3 1.0 1.0 0.2)


