import cgi
import datetime
import urllib
import wsgiref.handlers
import os

from google.appengine.ext import db
from google.appengine.api import users
from google.appengine.ext import webapp
from google.appengine.ext.webapp.util import run_wsgi_app
from google.appengine.ext.webapp import template


class File(db.Model):
  owner = db.UserProperty()
  filename = db.StringProperty()
  data = db.BlobProperty()


class MainPage(webapp.RequestHandler):
  def get(self):
    # Require the user to login.
    user = users.get_current_user()
    if not user:
      self.redirect(users.create_login_url(self.request.uri))
      return

    template_values = {
    }
    #path = os.path.join(os.path.dirname(__file__), 'index.html')
    path = os.path.join(os.path.dirname(__file__), 'hello_world.html')
    self.response.out.write(template.render(path, template_values))


def FileKey(owner, filename):
  return Key.from_path('File', ('%s_%s') % (owner.user_id(), filename))


class FileHandlingPage(webapp.RequestHandler):
  def post(self):
    # The user must be logged in.
    user = users.get_current_user()
    if not user:
      assert False

    method = self.request.path.rsplit('/', 1)[1]

    self.response.headers['Content-Type'] = 'application/octet-stream'
    if method == 'read':
      self.response.out.write('1')
      return
      filename = self.request.get('filename')
      assert filename
      k = FileKey(user, filename)
      f = File.get(k)
      if f:
        self.response.out.write('1')
        self.response.out.write(f.data)
      else:
        self.response.out.write('0')

    elif method == 'write':
      filename = self.request.get('filename')
      data = self.request.get('data')
      assert filename
      assert data
      def create_or_update(filename, data):
        k = FileKey(user, filename)
        f = File.get(k)
        if not f:
          f = File()
          f.owner = owner
          f.filename = filename
        f.data = data
        f.put()
      db.run_in_transaction(create_or_update, filename, data) 

    elif method == 'list':
      prefix = self.request.get('prefix')
      assert prefix
      q = File.all()
      q.filter('owner =', user)
      q.filter('filename >', prefix)
      q.filter('filename <', prefix + '\uffff')
      results = q.fetch(limit=100)
      for r in results:
        self.response.out.write('%s\n' % r.filename)

    elif method == 'remove':
      filename = self.request.get('filename')
      assert filename
      k = FileKey(user, filename)
      try:
        db.delete(k)
        self.response.out.write('1')
      except:
        self.response.out.write('0')

    else:
      assert False


application = webapp.WSGIApplication([
  ('/', MainPage),
  ('/_file/.*', FileHandlingPage),

], debug=True)


def main():
  run_wsgi_app(application)


if __name__ == '__main__':
  main()
