---
layout: default
link_to_home: true
---
{% include globjects-profile.md %}

<h2>Available Documentations</h2>

<div class="container">
  <div class="row">
    {% for s in site.documentations %}
      {% assign contextualcolor = 'secondary' %}
      {% if s.status == 'stable' %}{% assign contextualcolor = 'primary' %}{% endif %}
      {% if s.status == 'latest stable' %}{% assign contextualcolor = 'success' %}{% endif %}
      {% if s.status == 'unstable' %}{% assign contextualcolor = 'warning' %}{% endif %}
      <div class="col-lg-3 col-md-4 col-sm-6 mb-4">
        <div class="card card-outline-{{ contextualcolor }}">
          <a class="nav-link active" href="{{ site.baseurl }}/docs/{{ s.name }}">
          <div class="card-block text-center">
            <div class="pull-left">
              <h2>{{ s.name }}</h2>
            </div>
            <div class="pull-right">
              <span class="badge badge-{{ contextualcolor }}">{{ s.status }}</span>
            </div>
          </div>
          </a>
        </div>
      </div>
    {% endfor %}
  </div>
</div>

#### Technical integration guide

If you want to add a new or updated documentation for a specific release of {{ site.title }}, please refer to the [integration guide]({{ site.baseurl }}/docs/integration-guide.html).
