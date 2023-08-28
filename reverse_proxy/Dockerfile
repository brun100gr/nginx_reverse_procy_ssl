FROM nginx:latest

# Install apache2-utils to get htpasswd command
RUN apt-get update -y && apt-get install -y apache2-utils && rm -rf /var/lib/apt/lists/*

# default conf for proxy service
COPY ./default.conf /etc/nginx/conf.d/default.conf

# Domains conf
COPY ./sites-available /etc/nginx/sites-available/
COPY ./sites-enabled /etc/nginx/sites-enabled/
RUN ln -s /etc/nginx/sites-available/brunomarelli.duckdns.org.conf /etc/nginx/sites-enabled/
#RUN ln -s /etc/nginx/sites-available/brunomarelli01.duckdns.org.conf /etc/nginx/sites-enabled/

# NOT FOUND response
COPY ./backend-not-found.html /var/www/html/backend-not-found.html

# Proxy configurations
COPY ./includes/ /etc/nginx/includes/

# nginx config file
COPY ./nginx.conf /etc/nginx/nginx.conf

# .htpasswd
COPY ./.htpasswd /etc/nginx/.htpasswd
