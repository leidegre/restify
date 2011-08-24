
Security nag
    
    netsh http add urlacl url=http://+:80/restify user=.\user

    netsh http delete urlacl url=http://+:80/restify